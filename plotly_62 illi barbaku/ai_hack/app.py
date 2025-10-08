import chainlit as cl
import xml.etree.ElementTree as ET
import graphviz
import base64

last_uploaded_file = {}
last_generated_xml = {}
visualize_input_file = False
visualize_delta_file = True

if 0:
   missing_xml = '''<?xml version="1.0" encoding="UTF-8"?>
      <gate id="G4" type="AND">
         <output>E3</output>
         <input>E6</input>
         <input>E7</input>
      </gate>
   '''
else:   
   missing_xml = '''<gate id="G7" type="OR">
         <output>E3</output>
         <input>E6</input>
         <input>E7</input>
      </gate>
   '''

async def generate_missing_data(file):
   return missing_xml

async def visualize_xml(file):
   print("Inside visualize_xml")
   file_name = 'LLM Output'
   
   root = ET.fromstring(file)
   
   # Create Graphviz diagram
   dot = graphviz.Digraph(
      graph_attr={"rankdir": "LR", "bgcolor": "transparent"},
      node_attr={"style": "filled", "fillcolor": "#FFFFFF", "shape": "box"}
   )
   
   def build_graph(element, parent=None):
      node_id = str(id(element))
      label = f"<b>{element.tag}</b>"
      if element.attrib:
            label += "<br/>" + "<br/>".join(
               f"<font face='Courier'>{k}:</font> {v}" 
               for k, v in element.attrib.items()
            )
      dot.node(node_id, f"<{label}>", shape="none")
      
      if parent:
            dot.edge(parent, node_id)
            
      for child in element:
            build_graph(child, node_id)
   
   build_graph(root)
   
   # Generate and display SVG
   svg = dot.pipe(format="svg")
   svg_b64 = base64.b64encode(svg).decode("utf-8")
   
   await cl.Message(
      content=f"Visualization:",
      elements=[
            cl.Image(
               url=f"data:image/svg+xml;base64,{svg_b64}",
               name="xml_viz",
               display="inline",
               size="large"
            )
      ]
   ).send()
   
   # Ask user if the graph is okay using actions (buttons)
   actions = [
      cl.Action(name="yes", label="Yes", payload={"value": "yes"}),
      cl.Action(name="no", label="No", payload={"value": "no"})
   ]
   await cl.Message(
      content="Is the displayed graph okay?",
      actions=actions
   ).send()
      

@cl.on_chat_start
async def main():
   print("Inside main")
   await handle_upload()

async def handle_upload():
   print("Inside handle_upload")
   files = await cl.AskFileMessage(
      content="📤 Upload XML file (or click anywhere to cancel)",
      accept={"text/xml": [".xml"]},
      max_size_mb=10,
      raise_on_timeout=False
   ).send()
   
   if files:
      last_uploaded_file["file"] = files[0]
      
      with open(files[0].path, "r", encoding="utf-8") as f:         
         if visualize_input_file:
            await visualize_xml(f.read())   
         missing_data = await generate_missing_data(f)     
      
      if visualize_delta_file:   
         await visualize_xml(missing_data)
      
async def merge_data():
   pass

@cl.action_callback("yes")
async def yes_callback(action):
   print("Inside yes_callback")
   input_xml = last_uploaded_file.get("file")
   
   await merge_data()
   
    # Create a File element
   file_element = cl.File(
      name="products.xml",
      content=missing_xml.encode("utf-8"),  # must be bytes
      display="inline",  # or "download" for a download button
      mime="application/xml"
   )
   
   # Send the message with the file element
   await cl.Message(content="Here is your XML file!", elements=[file_element]).send()   
   
   await cl.Message(content="Great! You can upload another XML file if you want.").send()
   await handle_upload()

@cl.action_callback("no")
async def no_callback(action):
   print("Inside no_callback")
   file = last_uploaded_file.get("file")
   if file:
      await cl.Message(content="Re-visualizing the last uploaded XML file...").send()      
      with open(file.path, "r", encoding="utf-8") as f:
         missing_data = await generate_missing_data(f) 
         await visualize_xml(missing_data)
   else:
      await cl.Message(content="No file found to re-visualize. Please upload again.").send()
      await handle_upload()

@cl.on_message
async def handle_message(message: cl.Message):
   print("Inside handle_message")
   await handle_upload()
