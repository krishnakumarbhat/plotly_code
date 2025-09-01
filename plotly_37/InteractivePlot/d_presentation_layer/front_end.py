# This code is structured to generate the binary for the application.

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
    <title>Interactive Plots</title>
    <script rel="preload" src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <style>{minfy_css}</style>
    <script>{minfy_js}</script>
</head>
""" + """
<body>


    <h1>Interactive Plots</h1>
    <div class="tabs">{{TABS}}</div>
    {{CONTENT}}
    <footer>Copyright @ APTIV-2025</footer>
</body>
</html>"""