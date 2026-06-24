import json
import logging
import os
from pathlib import Path
from typing import Any, Dict, List, Optional
from datetime import datetime

logger = logging.getLogger(__name__)

LLAMA_SERVER_URL = os.environ.get(
    'LLAMA_SERVER_BASE_URL',
    os.environ.get('RAG_LLAMA_URL', 'http://127.0.0.1:8081'),
).rstrip('/')


class JiraIntegration:
    def __init__(self):
        self.base_url = os.environ.get('JIRA_BASE_URL', '').rstrip('/')
        self.pat = os.environ.get('JIRA_PAT', '')
        self.user = os.environ.get('JIRA_USER', '')
        self.api_token = os.environ.get('JIRA_API_TOKEN', '')
        self.default_project = os.environ.get('JIRA_DEFAULT_PROJECT', '')
        self.default_board_id = os.environ.get('JIRA_DEFAULT_BOARD_ID', '')
        self._enabled = bool(self.base_url and (self.pat or (self.user and self.api_token)))
        if not self._enabled:
            logger.warning('Jira not configured: set JIRA_BASE_URL and JIRA_PAT (or JIRA_USER + JIRA_API_TOKEN)')

    def _headers(self) -> Dict[str, str]:
        if self.pat:
            auth = self.pat
        else:
            import base64
            auth = base64.b64encode(f'{self.user}:{self.api_token}'.encode()).decode()
        return {
            'Authorization': f'Basic {auth}',
            'Content-Type': 'application/json',
        }

    def _post(self, url: str, data: dict) -> Optional[Dict[str, Any]]:
        if not self._enabled:
            return None
        try:
            import requests
            resp = requests.post(url, headers=self._headers(), json=data, timeout=30)
            resp.raise_for_status()
            return resp.json()
        except Exception as e:
            logger.error(f'Jira API error: {e}')
            return None

    def create_kpi_ticket(
        self,
        accuracy_score: float,
        log_path: str,
        hdf_path: str,
        html_path: str,
        details_dict: Dict[str, Any],
        sensor_id: str = '',
    ) -> Optional[str]:
        if accuracy_score >= 60:
            return None
        debug_analysis = self._debug_hdf5_analysis(accuracy_score, hdf_path, log_path)
        description = self._build_ticket_description(accuracy_score, log_path, hdf_path, debug_analysis)
        name = details_dict.get('name', sensor_id or 'unknown')
        summary = f'[KPI Alert] Low accuracy: {accuracy_score:.1f}% - {name}'
        return self._create_ticket(summary, description)

    def create_rag_description(self, html_path: str, rag_answer: str) -> Optional[str]:
        summary = f'[RAG Report] Analysis from {os.path.basename(html_path)}'
        return self._create_ticket(summary, rag_answer)

    def _create_ticket(self, summary: str, description: str) -> Optional[str]:
        if not self._enabled:
            return None
        url = f'{self.base_url}/rest/api/2/issue'
        data = {
            'fields': {
                'project': {'key': self.default_project},
                'summary': summary,
                'description': description,
                'issuetype': {'name': 'Task'},
            }
        }
        result = self._post(url, data)
        if result:
            key = result.get('key', '')
            logger.info(f'Created Jira ticket {key}')
            return key
        return None

    def find_low_accuracy_logs(self, accuracy_dict: Dict[str, float]) -> List[str]:
        return [k for k, v in accuracy_dict.items() if v < 60]

    def _build_ticket_description(self, accuracy_score: float, log_path: str, hdf_path: str, debug_analysis: str = '') -> str:
        now = datetime.now().isoformat()
        parts = [
            f'KPI Accuracy Alert',
            f'',
            f'Accuracy: {accuracy_score:.1f}%',
            f'Generated: {now}',
            f'Log file: {log_path}',
            f'HDF file: {hdf_path}',
            f'',
        ]
        if debug_analysis:
            parts.append(f'AI Debug Analysis:')
            parts.append(f'')
            parts.append(debug_analysis)
            parts.append(f'')
        parts.append(f'Accuracy is below the 60% threshold. Review the AI analysis above and the logs for root cause.')
        return '\n'.join(parts)

    def _summarize_hdf5(self, hdf_path: str) -> str:
        try:
            import h5py
            import numpy as np
        except ImportError:
            return ''

        path = Path(hdf_path)
        if not path.exists():
            return f'HDF5 file not found: {hdf_path}'

        try:
            f = h5py.File(str(path), 'r')
        except Exception as e:
            return f'Cannot open HDF5: {e}'

        lines = [f'HDF5 file: {path.name}', f'File size: {path.stat().st_size / 1024 / 1024:.1f} MB', '']

        def _walk_group(group, depth=0):
            indent = '  ' * depth
            for key in group:
                try:
                    item = group[key]
                except Exception:
                    continue
                if isinstance(item, h5py.Group):
                    lines.append(f'{indent}[{key}]')
                    _walk_group(item, depth + 1)
                elif isinstance(item, h5py.Dataset):
                    shape = str(item.shape) if item.shape is not None else 'scalar'
                    dtype = str(item.dtype) if item.dtype is not None else 'unknown'
                    lines.append(f'{indent}{key}: shape={shape}, dtype={dtype}')
                    if item.ndim == 1 and item.size > 0 and item.size <= 20:
                        try:
                            values = item[:]
                            if np.issubdtype(item.dtype, np.floating) or np.issubdtype(item.dtype, np.integer):
                                lines.append(f'{indent}  values={values.tolist()}')
                        except Exception:
                            pass
                    elif item.ndim == 1 and item.size > 20:
                        try:
                            arr = item[:]
                            if np.issubdtype(item.dtype, np.floating):
                                lines.append(f'{indent}  range=[{float(arr.min()):.4f}, {float(arr.max()):.4f}], mean={float(arr.mean()):.4f}')
                            elif np.issubdtype(item.dtype, np.integer):
                                lines.append(f'{indent}  range=[{int(arr.min())}, {int(arr.max())}], mean={float(arr.mean()):.2f}')
                        except Exception:
                            pass

        try:
            _walk_group(f)
        except Exception as e:
            lines.append(f'(error walking HDF: {e})')
        f.close()
        return '\n'.join(lines[:120])

    def _call_llm_for_debug(self, prompt: str) -> str:
        try:
            import requests
        except ImportError:
            return ''

        url = f'{LLAMA_SERVER_URL}/v1/chat/completions'
        payload = {
            'model': 'local',
            'messages': [
                {
                    'role': 'system',
                    'content': (
                        'You are a radar/KPI debug analyst. You will receive HDF5 data summaries '
                        'from radar validation runs. Analyze the data, identify potential causes '
                        'for low accuracy, and suggest specific things to investigate. '
                        'Be concise and technical. Focus on: scan count mismatches, '
                        'missing detections, signal range anomalies, or empty groups.'
                    ),
                },
                {'role': 'user', 'content': prompt},
            ],
            'temperature': 0.1,
            'max_tokens': 512,
            'stream': False,
        }

        try:
            resp = requests.post(url, json=payload, timeout=120)
            resp.raise_for_status()
            body = resp.json()
            choices = body.get('choices') or []
            if choices:
                msg = choices[0].get('message') or {}
                content = msg.get('content') or ''
                return content.strip()
        except Exception as e:
            logger.warning('LLM debug call failed: %s', e)
        return ''

    def _debug_hdf5_analysis(self, accuracy: float, hdf_path: str, log_path: str) -> str:
        summary = self._summarize_hdf5(hdf_path)
        if not summary:
            return ''

        prompt_lines = [
            f'A KPI validation run achieved only {accuracy:.1f}% accuracy.',
            f'Here is the HDF5 input data summary:',
            f'',
            summary,
            f'',
            f'Log path: {log_path}',
            f'',
            f'Analyze why accuracy might be low. Consider:',
            f'- Are there empty groups or zero-size datasets?',
            f'- Is the scan count very low?',
            f'- Are signal ranges abnormal?',
            f'- Could there be a timestamp/alignment mismatch?',
            f'- Are expected sensors/groups missing?',
            f'Provide specific debugging suggestions.',
        ]

        return self._call_llm_for_debug('\n'.join(prompt_lines))
