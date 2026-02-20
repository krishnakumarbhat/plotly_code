"""Plotly HTML generation helpers."""

from typing import Any, Dict, List, Sequence

import numpy as np
import plotly.express as px
import plotly.graph_objects as go
from plotly.offline import plot
from plotly.subplots import make_subplots


class KpiHtmlGen:
    """Generates Plotly graphs and wraps them into a single HTML report."""

    def __init__(self, template: str | None = None):
        self._template = template

    def scatter_plot(
        self, x: Sequence, y: Sequence, title: str, xaxis: str, yaxis: str
    ) -> str:
        fig = px.scatter(x=x, y=y, title=title, labels={"x": xaxis, "y": yaxis})
        fig.update_layout(template=self._template)
        return self._div(fig)

    def line_multi(
        self,
        x: Sequence,
        series: Dict[str, Sequence],
        title: str,
        xaxis: str,
        yaxis: str,
    ) -> str:
        fig = go.Figure()
        for name, y in series.items():
            fig.add_trace(
                go.Scatter(
                    x=list(x),
                    y=self._clean_series(y),
                    mode="lines+markers",
                    name=name,
                    marker=dict(size=5),
                    line=dict(width=2),
                )
            )
        fig.update_layout(
            title=title,
            xaxis_title=xaxis,
            yaxis_title=yaxis,
            template=self._template,
            hovermode="x unified",
            margin=dict(l=50, r=20, t=60, b=45),
        )
        return self._div(fig)

    def heatmap(
        self, z: np.ndarray, x_labels, y_labels, title: str, xaxis: str, yaxis: str
    ) -> str:
        fig = go.Figure(
            go.Heatmap(
                z=z,
                x=x_labels,
                y=y_labels,
                colorscale="Viridis",
                colorbar_title="Value",
            )
        )
        fig.update_layout(
            title=title, xaxis_title=xaxis, yaxis_title=yaxis, template=self._template
        )
        return self._div(fig)

    def detection_overlay_all(
        self,
        scan_index: np.ndarray,
        signal_2d: np.ndarray,
        signal_name: str,
        sensor_name: str,
    ) -> str:
        n_scans, n_dets = signal_2d.shape
        xs = np.repeat(scan_index, n_dets)
        ys = signal_2d.ravel()
        mask = ~np.isnan(ys)
        fig = go.Figure()
        fig.add_trace(
            go.Scattergl(
                x=xs[mask],
                y=ys[mask],
                mode="markers",
                marker=dict(size=2, opacity=0.3),
                name="All",
            )
        )
        med = np.nanmedian(signal_2d, axis=1)
        mm = ~np.isnan(med)
        fig.add_trace(
            go.Scatter(
                x=scan_index[mm],
                y=med[mm],
                mode="lines",
                name="Median",
                line=dict(color="red", width=2),
            )
        )
        fig.update_layout(
            title=f"{sensor_name} — {signal_name} (all detections)",
            xaxis_title="Scan Index",
            yaxis_title=signal_name,
            template=self._template,
        )
        return self._div(fig)

    def detection_heatmap(
        self,
        scan_index: np.ndarray,
        signal_2d: np.ndarray,
        signal_name: str,
        sensor_name: str,
    ) -> str:
        det_labels = [f"{i + 1:03d}" for i in range(signal_2d.shape[1])]
        return self.heatmap(
            signal_2d,
            det_labels,
            scan_index.tolist(),
            f"{sensor_name} — {signal_name} Heatmap",
            "Detection Index",
            "Scan Index",
        )

    def detection_box_by_signal(
        self,
        signal_2d: np.ndarray,
        signal_name: str,
        sensor_name: str,
        sample_dets: int = 20,
    ) -> str:
        fig = go.Figure()
        n_dets = signal_2d.shape[1]
        step = max(1, n_dets // sample_dets)
        for det_idx in range(0, n_dets, step):
            valid = signal_2d[:, det_idx]
            valid = valid[~np.isnan(valid)]
            if len(valid):
                fig.add_trace(go.Box(y=valid, name=f"{det_idx + 1:03d}"))
        fig.update_layout(
            title=f"{sensor_name} — {signal_name} Distribution",
            xaxis_title="Detection Index",
            yaxis_title=signal_name,
            template=self._template,
            showlegend=False,
        )
        return self._div(fig)

    def alignment_scatter(
        self,
        scan_index: np.ndarray,
        az_values: np.ndarray,
        el_values: np.ndarray,
        sensor_name: str,
    ) -> str:
        fig = make_subplots(
            rows=2, cols=1, shared_xaxes=True, subplot_titles=["Azimuth", "Elevation"]
        )
        maz, mel = ~np.isnan(az_values), ~np.isnan(el_values)
        fig.add_trace(
            go.Scatter(
                x=scan_index[maz],
                y=az_values[maz],
                mode="lines+markers",
                name="Azimuth",
                marker=dict(size=3),
            ),
            row=1,
            col=1,
        )
        fig.add_trace(
            go.Scatter(
                x=scan_index[mel],
                y=el_values[mel],
                mode="lines+markers",
                name="Elevation",
                marker=dict(size=3),
                line=dict(color="orange"),
            ),
            row=2,
            col=1,
        )
        fig.update_layout(
            title=f"{sensor_name} — Alignment Status",
            template=self._template,
            height=600,
        )
        fig.update_xaxes(title_text="Scan Index", row=2, col=1)
        fig.update_yaxes(title_text="Azimuth (rad)", row=1, col=1)
        fig.update_yaxes(title_text="Elevation (rad)", row=2, col=1)
        return self._div(fig)

    def comparison_scatter(
        self,
        scan_index: np.ndarray,
        input_signal: np.ndarray,
        output_signal: np.ndarray,
        signal_name: str,
        sensor_name: str,
        det_idx: int = 1,
    ) -> str:
        col = det_idx - 1
        in_col = input_signal[:, col] if col < input_signal.shape[1] else np.array([])
        out_col = (
            output_signal[:, col] if col < output_signal.shape[1] else np.array([])
        )
        fig = go.Figure()
        if len(in_col):
            m = ~np.isnan(in_col)
            fig.add_trace(
                go.Scatter(
                    x=scan_index[m],
                    y=in_col[m],
                    mode="lines+markers",
                    name="Input",
                    marker=dict(size=3),
                )
            )
        if len(out_col):
            m = ~np.isnan(out_col)
            fig.add_trace(
                go.Scatter(
                    x=scan_index[m],
                    y=out_col[m],
                    mode="lines+markers",
                    name="Output",
                    marker=dict(size=3),
                )
            )
        fig.update_layout(
            title=f"{sensor_name} — {signal_name} Det#{det_idx:03d} (Input vs Output)",
            xaxis_title="Scan Index",
            yaxis_title=signal_name,
            template=self._template,
        )
        return self._div(fig)

    def header_num_detections(
        self, scan_index: np.ndarray, num_valid: np.ndarray, sensor_name: str
    ) -> str:
        fig = go.Figure(
            go.Bar(
                x=scan_index.tolist(), y=num_valid.tolist(), marker_color="steelblue"
            )
        )
        fig.update_layout(
            title=f"{sensor_name} — Valid Detections per Scan",
            xaxis_title="Scan Index",
            yaxis_title="Num Valid Detections",
            template=self._template,
        )
        return self._div(fig)

    def stats_table(self, title: str, headers: List[str], rows: List[List[str]]) -> str:
        th = "".join(f"<th>{h}</th>" for h in headers)
        trs = []
        for r in rows:
            tds = "".join(f"<td>{c}</td>" for c in r)
            trs.append(f"<tr>{tds}</tr>")
        return f'<h3>{title}</h3><div class="scroll"><table><thead><tr>{th}</tr></thead><tbody>{"".join(trs)}</tbody></table></div>'

    def wrap_html(self, body_divs: List[str], title: str = "CAN KPI Report") -> str:
        parts = [
            "<!DOCTYPE html>",
            "<html><head>",
            '<meta charset="utf-8"/>',
            f"<title>{title}</title>",
            '<link rel="preconnect" href="https://cdn.plot.ly">',
            '<script src="https://cdn.plot.ly/plotly-latest.min.js"></script>',
            "<style>",
            "body{font-family:Segoe UI,Arial,sans-serif;margin:16px;background:#fafafa;}",
            ".plot-section{margin:16px 0;padding:12px;border:1px solid #e5e7eb;border-radius:10px;background:#fff;box-shadow:0 1px 2px rgba(0,0,0,.04);opacity:0;transform:translateY(6px);}",
            "body.loaded .plot-section{opacity:1;transform:none;transition:opacity .35s ease,transform .35s ease;}",
            "html{scroll-behavior:smooth;}",
            ".tables-container{display:flex;flex-wrap:wrap;gap:12px;}",
            ".table-wrapper{flex:1 1 300px;max-width:32%;border:1px solid #ccc;padding:6px;}",
            ".scroll{max-height:300px;overflow:auto;}",
            "table{border-collapse:collapse;width:100%;font-size:12px;}",
            "th,td{border:1px solid #999;padding:4px;text-align:right;}",
            "th{background:#5b9460;position:sticky;top:0;color:white;}",
            "h1{text-align:left;}",
            "</style>",
            "<script>window.addEventListener('DOMContentLoaded',()=>document.body.classList.add('loaded'));</script>",
            "</head><body>",
            f"<h1>{title}</h1>",
        ]
        for div in body_divs:
            parts.append('<div class="plot-section">')
            parts.append(div)
            parts.append("</div>")
        parts.append("</body></html>")
        return "\n".join(parts)

    def _div(self, fig) -> str:
        fig.update_layout(template=self._template)
        config = {"responsive": True, "displayModeBar": False}
        return plot(fig, include_plotlyjs=False, output_type="div", config=config)

    def _clean_series(self, y: Sequence[Any]) -> List[Any]:
        out: List[Any] = []
        for v in list(y):
            if v is None:
                out.append(None)
            else:
                try:
                    out.append(None if v != v else v)
                except Exception:
                    out.append(v)
        return out
