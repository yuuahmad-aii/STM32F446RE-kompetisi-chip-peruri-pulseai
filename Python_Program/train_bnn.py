import os
import cv2
import numpy as np
import tensorflow as tf

# =============================================================================
# KONFIGURASI KUANTISASI (C-Style Define)
# Pilihan: 'BNN', 'INT4', 'INT8', 'FP8'
# =============================================================================
QUANT_MODE = 'BNN'

# =============================================================================
# IMPLEMENTASI PURE TENSORFLOW UNTUK KUANTISASI (QAT)
# =============================================================================
@tf.custom_gradient
def ste_quantize(x):
    """
    Fungsi Aktivasi dengan Straight-Through Estimator (STE).
    Menyesuaikan mode kuantisasi berdasarkan define QUANT_MODE.
    """
    if QUANT_MODE == 'BNN':
        out = tf.where(x >= 0, 1.0, -1.0)
    elif QUANT_MODE == 'INT4':
        scale = 7.0 # 2^(4-1) - 1
        out = tf.round(tf.clip_by_value(x, -1.0, 1.0) * scale) / scale
    elif QUANT_MODE == 'INT8':
        scale = 127.0 # 2^(8-1) - 1
        out = tf.round(tf.clip_by_value(x, -1.0, 1.0) * scale) / scale
    elif QUANT_MODE == 'FP8':
        # Simulasi FP8 (proof of concept, menggunakan rentang dinamis serupa INT8)
        scale = 127.0
        out = tf.round(tf.clip_by_value(x, -1.0, 1.0) * scale) / scale
    else:
        out = x # Float32 (Tanpa kuantisasi)
    
    # Backward pass: Gradient = 1 jika |x| <= 1, selain itu 0 (Hard Tanh derivative)
    def grad(dy, variables=None):
        g = dy * tf.cast(tf.abs(x) <= 1.0, dy.dtype)
        if variables:
            return g, [None] * len(variables)
        return g
        
    return out, grad

class QuantizedDense(tf.keras.layers.Layer):
    def __init__(self, units, **kwargs):
        super(QuantizedDense, self).__init__(**kwargs)
        self.units = units

    def build(self, input_shape):
        # Bobot laten (desimal) yang akan diperbarui oleh Gradient Descent
        self.kernel = self.add_weight(
            shape=(input_shape[-1], self.units),
            initializer="glorot_normal",
            trainable=True,
            name="kernel"
        )
        # BNN menggunakan BatchNormalization sebagai pengganti Bias

    def call(self, inputs):
        # Kuantisasi Input dan Bobot secara on-the-fly saat Forward Pass
        q_inputs = ste_quantize(inputs)
        q_kernel = ste_quantize(self.kernel)
        
        # Perkalian Matriks
        return tf.matmul(q_inputs, q_kernel)

# =============================================================================

# Konfigurasi
DATASET_DIR = "dataset"
BATCH_SIZE = 32
EPOCHS = 60
MODEL_PATH = "bnn_model.h5"

print("Memuat dataset dari:", DATASET_DIR)

# --- GENERATE BACKGROUND/NOISE CLASS (Lainnya) ---
# noise_dir = os.path.join(DATASET_DIR, "lainnya")
# if not os.path.exists(noise_dir):
#     os.makedirs(noise_dir)
#     print("Membangkitkan data kelas negatif (Lainnya/Noise)...")
#     for i in range(100):
#         if i % 3 == 0:
#             img = np.random.randint(0, 256, (28, 28), dtype=np.uint8)
#         elif i % 3 == 1:
#             img = np.zeros((28, 28), dtype=np.uint8)
#         else:
#             img = np.zeros((28, 28), dtype=np.uint8)
#             x1, y1 = np.random.randint(0, 28, 2)
#             x2, y2 = np.random.randint(0, 28, 2)
#             cv2.line(img, (x1, y1), (x2, y2), int(np.random.randint(128, 256)), int(np.random.randint(1, 4)))
#         cv2.imwrite(os.path.join(noise_dir, f"noise_{i}.png"), img)
# -------------------------------------------------

# Memuat dataset (Grayscale, 28x28)
dataset = tf.keras.utils.image_dataset_from_directory(
    DATASET_DIR,
    labels='inferred',
    label_mode='categorical', # One-hot encoding
    color_mode='grayscale',
    batch_size=BATCH_SIZE,
    image_size=(28, 28),
    shuffle=True
)

class_names = dataset.class_names
NUM_CLASSES = len(class_names)
print(f"Ditemukan {NUM_CLASSES} kelas: {class_names}")

# --- DATA AUGMENTATION ---
data_augmentation = tf.keras.Sequential([
    tf.keras.layers.RandomTranslation(height_factor=0.15, width_factor=0.15, fill_mode='constant', fill_value=0),
    tf.keras.layers.RandomRotation(factor=0.2, fill_mode='constant', fill_value=0),
    tf.keras.layers.RandomBrightness(factor=0.3)
])

# Membagi dataset menjadi Train dan Validation (80% / 20%)
dataset_size = len(dataset)
train_size = int(0.8 * dataset_size)
if train_size == 0: train_size = 1 # Fallback jika data sangat sedikit

raw_train_dataset = dataset.take(train_size)
raw_val_dataset = dataset.skip(train_size)

# Normalisasi dan Binarisasi Input
def process_train(image, label):
    # Augmentasi hanya diaplikasikan pada training data
    image = data_augmentation(image, training=True)
    image = image / 255.0
    if QUANT_MODE == 'BNN':
        image = tf.where(image > 0.5, 1.0, -1.0)
    else:
        image = image * 2.0 - 1.0 # Petakan ke rentang [-1, 1]
    return image, label

def process_val(image, label):
    image = image / 255.0
    if QUANT_MODE == 'BNN':
        image = tf.where(image > 0.5, 1.0, -1.0)
    else:
        image = image * 2.0 - 1.0
    return image, label

# Terapkan augmentasi dan perbanyak data 10x lipat per epoch
train_dataset = raw_train_dataset.map(process_train).repeat(3)
val_dataset = raw_val_dataset.map(process_val)

# =============================================================================
# MEMBANGUN ARSITEKTUR BNN (MULTI-LAYER PERCEPTRON)
# =============================================================================

model = tf.keras.models.Sequential([
    tf.keras.layers.Flatten(input_shape=(28, 28, 1)),
    
    # Hidden Layer 1 (Diturunkan menjadi 64 agar muat di 8K LEs MAX10)
    QuantizedDense(64, name="bin_dense_1"),
    tf.keras.layers.BatchNormalization(scale=True, name="bn_1"),
    
    # Hidden Layer 2 (Diturunkan menjadi 32 agar muat di 8K LEs MAX10)
    QuantizedDense(32, name="bin_dense_2"),
    tf.keras.layers.BatchNormalization(scale=True, name="bn_2"),
    
    # Output Layer
    QuantizedDense(NUM_CLASSES, name="bin_dense_3"),
    tf.keras.layers.BatchNormalization(scale=True, name="bn_3"),
    tf.keras.layers.Activation("softmax")
])

print("\nArsitektur Model:")
model.summary()

# Scheduler Learning Rate (Cosine Decay) untuk stabilisasi training BNN
lr_schedule = tf.keras.optimizers.schedules.CosineDecay(
    initial_learning_rate=0.005,
    decay_steps=EPOCHS * 10,
    alpha=0.01
)

# Kompilasi
model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=lr_schedule),
    loss='categorical_crossentropy',
    metrics=['accuracy']
)

# Callback untuk menyimpan model dengan akurasi validasi tertinggi
checkpoint = tf.keras.callbacks.ModelCheckpoint(
    MODEL_PATH,
    monitor='val_accuracy',
    save_best_only=True,
    mode='max',
    verbose=1
)

# Training
print("\nMemulai Training QAT...")
model.fit(
    train_dataset,
    validation_data=val_dataset,
    epochs=EPOCHS,
    callbacks=[checkpoint]
)

# ModelCheckpoint sudah secara otomatis menyimpan model terbaik selama proses training.
print(f"\nModel terbaik telah otomatis disimpan ke {MODEL_PATH} oleh checkpoint!")
