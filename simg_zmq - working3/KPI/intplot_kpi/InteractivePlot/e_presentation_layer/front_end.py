# This code is structured to generate the binary for the application.

minfy_js = """document.addEventListener('DOMContentLoaded', function() {
    const body = document.body;
    const backdrop = document.getElementById('plot-backdrop');
    let expandedCard = null;

    function resizePlot(card) {
        if (!card || typeof Plotly === 'undefined') {
            return;
        }
        const graph = card.querySelector('.js-plotly-plot');
        if (graph) {
            requestAnimationFrame(function() {
                Plotly.Plots.resize(graph);
            });
        }
    }

    function closeExpandedPlot() {
        if (!expandedCard) {
            return;
        }
        expandedCard.classList.remove('expanded');
        const button = expandedCard.querySelector('.plot-expand-btn');
        if (button) {
            button.textContent = 'Expand';
            button.setAttribute('aria-expanded', 'false');
        }
        backdrop.classList.remove('visible');
        body.classList.remove('modal-open');
        resizePlot(expandedCard);
        expandedCard = null;
    }

    document.querySelectorAll('.plot-shell').forEach(function(card, idx) {
        card.style.animationDelay = (idx * 24) + 'ms';
        const button = card.querySelector('.plot-expand-btn');
        if (!button) {
            return;
        }
        button.addEventListener('click', function() {
            if (expandedCard === card) {
                closeExpandedPlot();
                return;
            }
            if (expandedCard) {
                closeExpandedPlot();
            }
            expandedCard = card;
            card.classList.add('expanded');
            button.textContent = 'Back';
            button.setAttribute('aria-expanded', 'true');
            backdrop.classList.add('visible');
            body.classList.add('modal-open');
            resizePlot(card);
        });
    });

    if (backdrop) {
        backdrop.addEventListener('click', closeExpandedPlot);
    }

    document.addEventListener('keydown', function(event) {
        if (event.key === 'Escape') {
            closeExpandedPlot();
        }
    });
});"""

minfy_css = """:root{--page-bg:#edf3f8;--panel-bg:#ffffff;--panel-border:#d7e3ee;--text-main:#16324a;--text-muted:#627486;--accent:#145f80;--accent-soft:#e9f4f9;--shadow:0 10px 26px rgba(17,44,68,.08)}*{box-sizing:border-box}html,body{margin:0;padding:0;background:radial-gradient(circle at top,#f9fcff 0%,var(--page-bg) 42%,#e7eef5 100%);color:var(--text-main);font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif}body{padding:22px}.modal-open{overflow:hidden}.page{max-width:1640px;margin:0 auto}.topbar{display:flex;justify-content:space-between;align-items:flex-start;gap:16px;margin-bottom:18px;flex-wrap:wrap}.hero{background:var(--panel-bg);border:1px solid var(--panel-border);border-radius:22px;box-shadow:var(--shadow);padding:22px 24px;flex:1 1 420px}.hero h1{margin:0 0 8px;font-size:2rem;letter-spacing:.01em}.hero p{margin:0;color:var(--text-muted);font-size:1.02rem}.info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px;flex:2 1 760px}.info-card{background:rgba(255,255,255,.82);backdrop-filter:blur(6px);border:1px solid var(--panel-border);border-radius:16px;padding:14px 16px;box-shadow:var(--shadow)}.info-card strong{display:block;font-size:.78rem;text-transform:uppercase;letter-spacing:.05em;color:var(--text-muted);margin-bottom:8px}.plot-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:18px;align-items:start}.plot-shell{position:relative;background:var(--panel-bg);border:1px solid var(--panel-border);border-radius:20px;box-shadow:var(--shadow);overflow:hidden;min-width:0;opacity:0;transform:translateY(10px);animation:fadeUp .28s ease-out forwards;transition:transform .22s ease,border-color .22s ease,box-shadow .22s ease}.plot-shell.expanded{position:fixed;inset:24px;z-index:1001;margin:0;transform:none;animation:none;border-color:#bdd6e4;box-shadow:0 26px 70px rgba(18,43,67,.22)}.plot-head{display:flex;justify-content:space-between;align-items:center;gap:12px;padding:14px 18px;border-bottom:1px solid #e8eef5;background:linear-gradient(180deg,#fcfeff 0%,#f3f8fb 100%)}.plot-head h4{margin:0;font-size:1.04rem;color:var(--accent);word-break:break-word}.plot-tools{display:flex;align-items:center;gap:8px;flex-shrink:0}.plot-expand-btn{appearance:none;border:1px solid #c8dce8;background:var(--accent-soft);color:var(--accent);border-radius:999px;padding:7px 12px;font-size:.86rem;font-weight:700;cursor:pointer;transition:background .16s ease,border-color .16s ease,transform .16s ease}.plot-expand-btn:hover{background:#dceef7;border-color:#b4d1e0}.plot-expand-btn:active{transform:translateY(1px)}.plot-body{padding:12px 12px 8px;min-height:420px}.plot-shell.expanded .plot-body{min-height:calc(100vh - 124px)}.plot-body>div{width:100%}.plot-body .js-plotly-plot{width:100%!important}.js-plotly-plot .plotly .modebar{right:10px!important;top:10px!important}.plot-backdrop{position:fixed;inset:0;background:rgba(16,34,50,.34);backdrop-filter:blur(4px);opacity:0;pointer-events:none;transition:opacity .18s ease;z-index:1000}.plot-backdrop.visible{opacity:1;pointer-events:auto}.page-nav{display:flex;justify-content:space-between;align-items:center;gap:12px;margin:18px 0 8px;flex-wrap:wrap}.page-nav a{color:var(--accent);text-decoration:none;font-weight:600;background:var(--accent-soft);border:1px solid #cfe0e8;border-radius:999px;padding:8px 14px;transition:background .16s ease,color .16s ease}.page-nav a:hover{background:#dcebf2}.page-nav .page-count{color:var(--text-muted);font-size:.95rem}.empty-state{background:var(--panel-bg);border:1px dashed var(--panel-border);border-radius:18px;padding:28px;text-align:center;color:var(--text-muted)}footer{text-align:center;color:#54708a;margin:22px 0 6px;font-size:.95rem}@keyframes fadeUp{from{opacity:0;transform:translateY(10px)}to{opacity:1;transform:translateY(0)}}@media (max-width:980px){body{padding:14px}.plot-grid{grid-template-columns:1fr}.plot-shell.expanded{inset:12px}.plot-body{min-height:360px}.plot-shell.expanded .plot-body{min-height:calc(100vh - 108px)}.plot-head{align-items:flex-start;flex-direction:column}}"""

html_template = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://cdn.plot.ly/plotly-3.0.1.min.js"></script>
    <script>{minfy_js}</script>
    <style>{minfy_css}</style>
</head>
<body>
    <div class="page">
        <div class="topbar">
            <section class="hero">
                <h1>Interactive Plots</h1>
                <p>{{{{SENSOR_POSITION}}}} · {{{{INPUT_FILENAME}}}}</p>
            </section>
            <section class="info-grid">
                <div class="info-card"><strong>Tool run on</strong><span>win32</span></div>
                <div class="info-card"><strong>Tool Version</strong><span>1.0</span></div>
                <div class="info-card"><strong>Generated</strong><span>{{{{GENERATION_TIME}}}}</span></div>
                <div class="info-card"><strong>Output File</strong><span>{{{{OUTPUT_FILENAME}}}}</span></div>
            </section>
        </div>
        {{{{TABS}}}}
        {{{{CONTENT}}}}
        <footer>APTIV Interactive Plot Report</footer>
    </div>
    <div id="plot-backdrop" class="plot-backdrop" aria-hidden="true"></div>
</body>
</html>"""