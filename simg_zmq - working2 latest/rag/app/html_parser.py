from __future__ import annotations

import re
from pathlib import Path

from bs4 import BeautifulSoup


class HtmlParser:
    def __init__(self, english_only: bool = True) -> None:
        self.english_only = english_only

    def parse_file(self, file_path: Path) -> str:
        html = file_path.read_text(encoding="utf-8", errors="ignore")
        return self.parse_html(html)

    def parse_html(self, html: str) -> str:
        soup = BeautifulSoup(html, "lxml")

        for tag in soup(["script", "style", "noscript", "svg", "footer", "nav", "header"]):
            tag.decompose()

        text = soup.get_text(separator=" ", strip=True)
        text = re.sub(r"\s+", " ", text).strip()
        if self.english_only:
            text = self._filter_english_text(text)
        return text

    @staticmethod
    def _filter_english_text(text: str) -> str:
        filtered = re.sub(r"[^A-Za-z0-9\s\.,;:!?@#%&\(\)\[\]\{\}\-_\\/\\'\"\+=\*<>\|`~$]", " ", text)
        filtered = re.sub(r"\s+", " ", filtered).strip()
        return filtered
