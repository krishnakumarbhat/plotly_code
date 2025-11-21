# Use Python 3.9 as base image
FROM python:3.9-slim

# Set working directory
WORKDIR /app

# Copy requirements first to leverage Docker cache
COPY requirements.txt .

# Install dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of the application
COPY . .

# Create directory for output if needed
RUN mkdir -p /app/output

# Set environment variables
ENV PYTHONUNBUFFERED=1

# Command to run the application
CMD ["python", "ResimHTMLReport.py"]