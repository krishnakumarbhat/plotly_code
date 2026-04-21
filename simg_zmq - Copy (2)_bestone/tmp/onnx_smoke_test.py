import json
import sys


def main() -> int:
    if len(sys.argv) != 3:
        print("Usage: python onnx_smoke_test.py <video_path> <video_root>")
        return 1

    sys.path.insert(0, "/app/Hyperlink_tool/code/html_online")

    import vlm

    result = vlm.process_video_with_vlm(sys.argv[1], sys.argv[2], force=True)
    print("RESULT_JSON=" + json.dumps(result, ensure_ascii=True))
    return 0 if result.get("success") else 2


if __name__ == "__main__":
    raise SystemExit(main())