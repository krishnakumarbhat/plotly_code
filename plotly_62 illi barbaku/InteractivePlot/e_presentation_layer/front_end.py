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
    <h1>Interactive Plots</h1>
    <h2>Click on "box select"<img src="data:image/png;base64,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"
    alt="|=|" style="width:79px; height:auto;"> to load the required plot</h2>
    <h3>Tool run on {sys.platform}</h3>
    <h3>Tool Version : 1.0</h3>
    <h3>HTML Report Generation Date Time Info : {datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</h3>
    <h3>input filename:</h3>
    <h3>ouput filename: </h3>

    <script rel="preload" src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <style>{minfy_css}</style>
    <script>{minfy_js}</script>
</head>
""" + """
<body>
    <div class="tabs">{{TABS}}</div>
    {{CONTENT}}
    <footer>Copyright @ APTIV-2025</footer>
</body>
</html>"""