# This code is structured to generate the binary for the application.
import sys
import datetime

minfy_js = """function showTab(t){
    console.log("Attempting to show tab:", t);
    const e = document.getElementsByClassName("tab-content");
    for (let n = 0; n < e.length; n++) e[n].style.display = "none";
    const o = document.getElementById(t);
    if (o) o.style.display = "block";
    else console.error(`Tab content with ID "${t}" not found.`);
    
    const a = document.getElementsByClassName("tab");
    for (let r = 0; r < a.length; r++) a[r].classList.remove("active-tab");
    const i = Array.from(a).find(e => e.innerText.toLowerCase().replace(" ", "-") === t.toLowerCase());
    if (i) i.classList.add("active-tab");
}
document.addEventListener('DOMContentLoaded', function() {
    showTab('Vehical');
});"""

minfy_css = """body{font-family:Arial,sans-serif;margin:20px}h1{text-align:center}.tabs{display:flex;justify-content:center;margin-bottom:20px}.tab{padding:10px 20px;cursor:pointer;border:1px solid #ccc;margin-right:5px;border-radius:15px}.active-tab{background-color:#8d30f7;color:white}table{width:100%;border-collapse:collapse}td{border:1px solid #ccc;padding:10px}footer{text-align:center;margin-top:20px}@media (max-width:600px){td{display:block;width:100%;margin-bottom:10px}}"""

html_template = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <script rel="preload" src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <style>{minfy_css}</style>
    <script>{minfy_js}</script>

    <h1>Interactive Plots</h1>
    <div id="lazyload-info" style="background-color: #f0f0f0; padding: 15px; border-radius: 10px; margin-bottom: 20px;">
        <h2>Click on "box select"<img src="data:image/png;base64,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" alt="|=|" style="width:79px; height:auto;"> to load the required plot</h2>
        <h4>NOTE:All blue plots don't have legend because they are completely matching</h4>
    </div>
    <h3>Tool run on {sys.platform}</h3>
    <h3>Tool Version : 1.0</h3>
    <h3>HTML  Generated Time Info : {datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</h3>
""" +"""
    <h3>Input Filename: <span id="input-filename">{{INPUT_FILENAME}}</span></h3>
    <h3>Output Filename: <span id="output-filename">{{OUTPUT_FILENAME}}</span></h3>
    <h3>Sensor Position: <span id="sensor-position">{{SENSOR_POSITION}}</span></h3>
</head>
<body>
    <div class="tabs">{{TABS}}</div>
    {{CONTENT}}
    <footer>Copyright @ APTIV-2025</footer>
</body>
</html>"""