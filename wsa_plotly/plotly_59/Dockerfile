FROM python:3.9-slim

WORKDIR /app

COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

COPY . .

RUN mkdir -p /app/output

ENV PYTHONUNBUFFERED=1

CMD ["python", "ResimHTMLReport.py"]


#singularity build my-app.simg Dockerfile is the command
