import numpy as np
import tensorflow as tf

# =============================================================================
# KONFIGURASI KUANTISASI (Harus sama dengan train_bnn.py)
# =============================================================================
QUANT_MODE = 'BNN'

# =============================================================================
# IMPLEMENTASI PURE TENSORFLOW UNTUK KUANTISASI (Sama seperti saat training)
# =============================================================================
@tf.custom_gradient
def ste_quantize(x):
    if QUANT_MODE == 'BNN':
        out = tf.where(x >= 0, 1.0, -1.0)
    elif QUANT_MODE == 'INT4':
        scale = 7.0
        out = tf.round(tf.clip_by_value(x, -1.0, 1.0) * scale) / scale
    elif QUANT_MODE == 'INT8' or QUANT_MODE == 'FP8':
        scale = 127.0
        out = tf.round(tf.clip_by_value(x, -1.0, 1.0) * scale) / scale
    else:
        out = x

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
        self.kernel = self.add_weight(shape=(input_shape[-1], self.units), initializer="glorot_normal", trainable=True, name="kernel")
    def call(self, inputs):
        return tf.matmul(ste_quantize(inputs), ste_quantize(self.kernel))
# =============================================================================

MODEL_PATH = "bnn_model.h5"
OUTPUT_VH = "weights.vh"

print(f"Memuat model {MODEL_PATH}...")
model = tf.keras.models.load_model(MODEL_PATH, custom_objects={'QuantizedDense': QuantizedDense, 'ste_quantize': ste_quantize})

if QUANT_MODE != 'BNN':
    raise ValueError("Script ekstraksi FPGA ini dirancang HANYA untuk model BNN (1-bit).")

def extract_bnn_params(model, output_file):
    with open(output_file, 'w') as f:
        f.write("// ====================================================\n")
        f.write("// AUTO-GENERATED BNN WEIGHTS & THRESHOLDS FOR FPGA\n")
        f.write("// ====================================================\n\n")
        
        layer_idx = 1
        
        # Cari pasangan QuantizedDense dan BatchNormalization
        for i in range(len(model.layers)):
            layer = model.layers[i]
            
            if isinstance(layer, QuantizedDense):
                print(f"Memproses Layer {layer_idx}: {layer.name}")
                
                # 1. Ekstraksi Bobot Laten dan Binarisasi
                latent_weights = layer.get_weights()[0] # Shape: (in_features, out_features)
                in_features, out_features = latent_weights.shape
                
                # Binarisasi: W >= 0 -> 1, W < 0 -> 0
                # Di FPGA, XNOR akan mengubah bit 0 ini kembali menjadi -1 secara matematis
                binary_weights = np.where(latent_weights >= 0, 1, 0)
                
                # Cetak Parameter Bobot
                f.write(f"// --- LAYER {layer_idx} WEIGHTS ({in_features} inputs, {out_features} neurons) ---\n")
                for neuron in range(out_features):
                    # Format Verilog: 784'b10110...
                    bits = ''.join(str(b) for b in binary_weights[:, neuron])
                    # Membalik urutan bit (Endianness) jika diperlukan oleh RTL Anda.
                    # Kita asumsikan bit 0 adalah input[0], sehingga urutannya [in_features-1 : 0]
                    bits_reversed = bits[::-1]
                    f.write(f"parameter [{in_features-1}:0] L{layer_idx}_N{neuron}_W = {in_features}'b{bits_reversed};\n")
                
                f.write("\n")
                
                # 2. Ekstraksi Threshold dari BatchNormalization
                # Kita cari layer BatchNorm setelah QuantDense ini
                bn_layer = None
                if i + 1 < len(model.layers) and isinstance(model.layers[i+1], tf.keras.layers.BatchNormalization):
                    bn_layer = model.layers[i+1]
                
                if bn_layer:
                    gamma, beta, mean, variance = bn_layer.get_weights()
                    epsilon = bn_layer.epsilon
                    
                    f.write(f"// --- LAYER {layer_idx} POPCOUNT THRESHOLDS ---\n")
                    # Threshold = ceil( (N + mean - (beta * std / gamma)) / 2 )
                    for neuron in range(out_features):
                        g = gamma[neuron]
                        b = beta[neuron]
                        m = mean[neuron]
                        v = variance[neuron]
                        
                        std = np.sqrt(v + epsilon)
                        
                        if g == 0: g = 1e-7 # Mencegah pembagian dengan nol
                        
                        # Kalkulasi threshold popcount
                        thresh_float = (in_features + m - (b * std / g)) / 2.0
                        
                        # Jika gamma negatif, arah pertidaksamaan berubah, tapi dalam praktek jarang terjadi
                        if g < 0:
                            thresh_float = in_features - thresh_float
                            
                        thresh_int = int(np.ceil(thresh_float))
                        
                        # Batasi nilai threshold agar tidak over/underflow
                        thresh_int = max(0, min(in_features, thresh_int))
                        
                        f.write(f"parameter int L{layer_idx}_N{neuron}_T = {thresh_int};\n")
                        
                f.write("\n\n")
                layer_idx += 1

extract_bnn_params(model, OUTPUT_VH)
print(f"Berhasil diekstrak ke {OUTPUT_VH}")
