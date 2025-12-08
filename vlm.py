./llama-cli -m Qwen3-VL-2B-Instruct-Q5_K_M.gguf -p "Describe this image" --mmproj mmproj-model-f16.gguf --image test.jpg



import torch
from transformers import AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig
from qwen_vl_utils import process_vision_info

# 1. Load Model without explicit quantization (let device_map handle offloading if needed)
# Quantization was causing issues with Qwen3-VL custom layers.
# 2B model in FP16 is ~4GB, might fit or offload.

model_id = "Qwen/Qwen3-VL-2B-Instruct"

print("Loading model...")
model = AutoModelForVision2Seq.from_pretrained(
    model_id,
    device_map="auto",
    torch_dtype=torch.float16,
    trust_remote_code=True
)

processor = AutoProcessor.from_pretrained(model_id, trust_remote_code=True)

# 2. Prepare Input
video_path = "video.mp4"
print(f"Processing {video_path}...")

# Calculate FPS to get approx 8 frames
import cv2
cap = cv2.VideoCapture(video_path)
total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
fps_source = cap.get(cv2.CAP_PROP_FPS)
duration = total_frames / fps_source
cap.release()

target_frames = 8
fps_sample = target_frames / duration
print(f"Video duration: {duration:.2f}s. Sampling at {fps_sample:.4f} FPS to get ~{target_frames} frames.")

messages = [
    {
        "role": "user",
        "content": [
            {
                "type": "video",
                "video": video_path,
                "max_pixels": 360 * 420, # Limit resolution to save VRAM
                "fps": fps_sample, 
            },
            {"type": "text", "text": "Describe this video in detail."},
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
inputs = inputs.to("cuda")

# 3. Generate
print("Generating description...")
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