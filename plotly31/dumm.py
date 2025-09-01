from flask import Flask, request, jsonify

app = Flask(__name__)

# Sample data to simulate a database
data_store = []

@app.route('/')
def home():
    name = request.args["name"]
    return "Welcome to the Dummy Flask App!{{ name }}"

# @app.route('/data', methods=['GET'])
# def get_data():
#     """Retrieve all data."""
#     return jsonify(data_store), 200

# @app.route('/data', methods=['POST'])
# def add_data():
#     """Add new data to the store."""
#     if request.is_json:
#         new_data = request.get_json()
#         data_store.append(new_data)
#         return jsonify({"message": "Data added successfully!", "data": new_data}), 201
#     return jsonify({"error": "Request must be JSON"}), 400

# @app.route('/data/<int:index>', methods=['GET'])
# def get_single_data(index):
#     """Retrieve a specific item by index."""
#     if index < len(data_store):
#         return jsonify(data_store[index]), 200
#     return jsonify({"error": "Data not found"}), 404

# @app.route('/data/<int:index>', methods=['DELETE'])
# def delete_data(index):
#     """Delete a specific item by index."""
#     if index < len(data_store):
#         deleted_item = data_store.pop(index)
#         return jsonify({"message": "Data deleted successfully!", "data": deleted_item}), 200
#     return jsonify({"error": "Data not found"}), 404

if __name__ == '__main__':
    app.run(debug=True)
