

import os
import ssl
import warnings

# Disable SSL verification BEFORE importing any other libraries
# This is needed for corporate proxies with self-signed certificates
os.environ['CURL_CA_BUNDLE'] = ''
os.environ['REQUESTS_CA_BUNDLE'] = ''
os.environ['HF_HUB_DISABLE_SSL_VERIFY'] = '1'
os.environ['TRANSFORMERS_OFFLINE'] = '0'

# Patch SSL globally
ssl._create_default_https_context = ssl._create_unverified_context

import urllib3
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

# Patch requests to disable SSL verification
import requests
from requests.adapters import HTTPAdapter

class SSLAdapter(HTTPAdapter):
    def init_poolmanager(self, *args, **kwargs):
        kwargs['ssl_context'] = ssl._create_unverified_context()
        return super().init_poolmanager(*args, **kwargs)

# Monkey-patch the default session
old_request = requests.Session.request
def new_request(self, *args, **kwargs):
    kwargs['verify'] = False
    return old_request(self, *args, **kwargs)
requests.Session.request = new_request

import torch
import sys
from transformers import AutoModelForImageTextToText, AutoProcessor
from qwen_vl_utils import process_vision_info

# Windows CPU-only version (Intel integrated graphics)
# Note: This will be slower than GPU but works without NVIDIA

# Use local model path for offline usage (no internet required)
# This folder can be pushed to git or shared within your company
import os
script_dir = os.path.dirname(os.path.abspath(__file__))
model_id = os.path.join(script_dir, "models", "Qwen3-VL-2B-Instruct")

# Fallback to HuggingFace if local model not found
if not os.path.exists(model_id):
    print("Local model not found, using HuggingFace (requires internet)...", flush=True)
    model_id = "Qwen/Qwen3-VL-2B-Instruct"
else:
    print(f"Using local model from: {model_id}", flush=True)

print("Loading model (CPU mode - this may take a few minutes)...", flush=True)

try:
    print("Step 1: Calling from_pretrained...", flush=True)
    model = AutoModelForImageTextToText.from_pretrained(
        model_id,
        device_map="cpu",
        trust_remote_code=True,
        low_cpu_mem_usage=True,  # Helps with memory on systems with limited RAM
        local_files_only=os.path.exists(model_id)  # Use offline mode if local model exists
    )
    # Convert to float32 after loading
    model = model.float()
    print("Model loaded successfully!", flush=True)
except Exception as e:
    print(f"Error loading model: {e}", flush=True)
    import traceback
    traceback.print_exc()
    sys.exit(1)

print("Loading processor...", flush=True)
try:
    processor = AutoProcessor.from_pretrained(
        model_id, 
        trust_remote_code=True,
        local_files_only=os.path.exists(model_id)
    )
    print("Processor loaded successfully!", flush=True)
except Exception as e:
    print(f"Error loading processor: {e}", flush=True)
    import traceback
    traceback.print_exc()
    sys.exit(1)

# 2. Prepare Input
video_path = "C:/Users/ouymc2/Desktop/dc_viz/db/video/dagfjsdhaf_3_ert.mp4"
print(f"Processing {video_path}...")

# Calculate FPS to get approx 4 frames (reduced for CPU)
import cv2
cap = cv2.VideoCapture(video_path)
total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
fps_source = cap.get(cv2.CAP_PROP_FPS)
duration = total_frames / fps_source
cap.release()

# Reduce frames for CPU to speed up processing
target_frames = 4
fps_sample = target_frames / duration
print(f"Video duration: {duration:.2f}s. Sampling at {fps_sample:.4f} FPS to get ~{target_frames} frames.")

messages = [
    {
        "role": "user",
        "content": [
            {
                "type": "video",
                "video": video_path,
                "max_pixels": 360 * 420,  # Must be >= min_pixels (default ~150k)
                "min_pixels": 128 * 128,  # Set min_pixels lower for CPU
                "fps": fps_sample, 
            },
            {"type": "text", "text": "Describe this video in detail. thi is senrio of adas host will be white and target will be in other colotr and thier will be gradrils and also truck,motorbike and radar detectionsjsut tell the "},
        ],
    }
]

# Preparation for inference
text = processor.apply_chat_template(
    messages, tokenize=False, add_generation_prompt=True
)
image_inputs, video_inputs = process_vision_info(messages)
inputs = processor(
    text=[text],
    images=image_inputs,
    videos=video_inputs,
    padding=True,
    return_tensors="pt",
)
# Keep on CPU (no .to("cuda"))

# 3. Generate
print("Generating description (CPU inference - please wait, this may take several minutes)...")
with torch.no_grad():  # Disable gradient computation for inference
    generated_ids = model.generate(**inputs, max_new_tokens=128)
generated_ids_trimmed = [
    out_ids[len(in_ids) :] for in_ids, out_ids in zip(inputs.input_ids, generated_ids)
]
output_text = processor.batch_decode(
    generated_ids_trimmed, skip_special_tokens=True, clean_up_tokenization_spaces=False
)

# 4. Save Output
print("Saving output...")
with open("output2.txt", "w") as f:
    f.write(output_text[0])

print("Done! Output saved to output2.txt")
print(f"Output: {output_text[0]}")