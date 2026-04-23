from __future__ import annotations

from html.parser import HTMLParser
import re
from pathlib import Path


class _HtmlTextExtractor(HTMLParser):
    def __init__(self) -> None:
        super().__init__()
        self._skip_depth = 0
        self._parts: list[str] = []

    def handle_starttag(self, tag: str, attrs) -> None:
        if tag.lower() in {"script", "style", "noscript", "svg", "footer", "nav", "header"}:
            self._skip_depth += 1

    def handle_endtag(self, tag: str) -> None:
        if tag.lower() in {"script", "style", "noscript", "svg", "footer", "nav", "header"} and self._skip_depth:
            self._skip_depth -= 1

    def handle_data(self, data: str) -> None:
        if self._skip_depth == 0:
            self._parts.append(data)

    def extract(self, html: str) -> str:
        self._skip_depth = 0
        self._parts = []
        self.feed(html)
        self.close()
        return " ".join(self._parts)


class HtmlParser:
    def __init__(self, english_only: bool = True) -> None:
        self.english_only = english_only
        self._extractor = _HtmlTextExtractor()

    def parse_file(self, file_path: Path) -> str:
        html = file_path.read_text(encoding="utf-8", errors="ignore")
        if file_path.suffix.lower() in {".html", ".htm", ".xml"}:
            return self.parse_html(html)
        return self._normalize_text(html)

    def parse_html(self, html: str) -> str:
        text = self._extractor.extract(html)
        return self._normalize_text(text)

    def _normalize_text(self, text: str) -> str:
        text = re.sub(r"\s+", " ", text).strip()
        if self.english_only:
            text = self._filter_english_text(text)
        return text

    @staticmethod
    def _filter_english_text(text: str) -> str:
        filtered = re.sub(r"[^A-Za-z0-9\s\.,;:!?@#%&\(\)\[\]\{\}\-_\\/\\'\"\+=\*<>\|`~$]", " ", text)
        filtered = re.sub(r"\s+", " ", filtered).strip()
        return filtered
