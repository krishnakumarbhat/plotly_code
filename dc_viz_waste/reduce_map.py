# python
# Requires: auto-gptq (pip), transformers, accelerate, safetensors
# pip install auto-gptq transformers accelerate safetensors
# this code does this "4-bit / GPTQ quantization
# Pros: often reduces size drastically (×4–×8), still usable for inference with specialized loaders.
# Cons: requires conversion tools, some accuracy loss, needs special runtime support.
# When to use: you want the full model but much smaller and still usable locally.
# 8-bit (bitsandbytes) for inference
# Pros: simple to use for inference; memory reduced at runtime.
# Cons: saving an 8-bit file usually still yields a large file; doesn’t always reduce on-disk weights unless you run a conversion tool.
# Convert to a GGML/GGUF (llama.cpp style) or other compact binary
# Pros: very small, fast on CPU, many converters exist for popular models.
# Cons: not every model is supported; conversion can be involved""  "


import os
import subprocess
from pathlib import Path

LOCAL_MODEL = r"c:\Users\ouymc2\Desktop\dc_viz\models\Qwen3-VL-2B-Instruct"  # local HF folder
OUT_DIR = r"c:\Users\ouymc2\Desktop\dc_viz\models\Qwen3-VL-2B-Instruct-4bit"

os.makedirs(OUT_DIR, exist_ok=True)

# 1) Run the AutoGPTQ conversion CLI (offline, uses local model files)
cmd = [
    "python", "-m", "auto_gptq.convert",
    "--model_name_or_path", LOCAL_MODEL,
    "--wbits", "4",
    "--save_dir", OUT_DIR,
    "--groupsize", "128",
    "--use_safetensors"
]
subprocess.run(cmd, check=True)

# 2) Example load of the quantized model (inference)
# Note: exact AutoGPTQ load API may vary by version; adjust if needed.
from transformers import AutoTokenizer
from auto_gptq import AutoGPTQForCausalLM  # auto_gptq library

tokenizer = AutoTokenizer.from_pretrained(LOCAL_MODEL, use_fast=True)
model = AutoGPTQForCausalLM.from_pretrained(
    OUT_DIR,
    device="cuda:0",          # change to "cpu" if no GPU
    use_safetensors=True,
    trust_remote_code=True
)

inputs = tokenizer("Describe this radar visualization:", return_tensors="pt").to(model.device)
out = model.generate(**inputs, max_length=128)
print(tokenizer.decode(out[0], skip_special_tokens=True))