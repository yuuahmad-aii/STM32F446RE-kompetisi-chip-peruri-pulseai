import os
import re

vh_file = 'weights.vh'

with open(vh_file, 'r') as f:
    content = f.read()

def parse_weights_and_thresholds(prefix, num_neurons, weight_width):
    weights = []
    thresholds = []
    for i in range(num_neurons):
        w_match = re.search(f'parameter\s+\[\d+:\d+\]\s+{prefix}_N{i}_W\s*=\s*\d+\'b([01]+);', content)
        t_match = re.search(f'parameter\s+int\s+{prefix}_N{i}_T\s*=\s*(-?\d+);', content)
        if w_match:
            weights.append(w_match.group(1))
        else:
            weights.append('0' * weight_width)
        
        if t_match:
            thresholds.append(int(t_match.group(1)))
        else:
            thresholds.append(0)
    return weights, thresholds

def write_mif(filename, width, depth, data, is_binary=True):
    with open(filename, 'w') as f:
        f.write(f"WIDTH={width};\n")
        f.write(f"DEPTH={depth};\n")
        f.write("ADDRESS_RADIX=UNS;\n")
        if is_binary:
            f.write("DATA_RADIX=BIN;\n")
        else:
            f.write("DATA_RADIX=DEC;\n")
        f.write("CONTENT BEGIN\n")
        for i, val in enumerate(data):
            f.write(f"\t{i} : {val};\n")
        
        if len(data) < depth:
            f.write(f"\t[{len(data)}..{depth-1}] : {'0'*width if is_binary else '0'};\n")
        f.write("END;\n")

# L1: 64 neurons, 784 weights
l1_w, l1_t = parse_weights_and_thresholds('L1', 64, 784)
write_mif('l1_w.mif', 784, 64, l1_w, is_binary=True)
write_mif('l1_t.mif', 32, 64, l1_t, is_binary=False)

# L2: 32 neurons, 64 weights
l2_w, l2_t = parse_weights_and_thresholds('L2', 32, 64)
write_mif('l2_w.mif', 64, 32, l2_w, is_binary=True)
write_mif('l2_t.mif', 32, 32, l2_t, is_binary=False)

# L3: 4 neurons, 32 weights
l3_w, l3_t = parse_weights_and_thresholds('L3', 4, 32)
write_mif('l3_w.mif', 32, 4, l3_w, is_binary=True)
write_mif('l3_t.mif', 32, 4, l3_t, is_binary=False)

print("MIF files generated successfully.")
