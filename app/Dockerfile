# Dockerfile para ESP8266 DHT22 Monitor
FROM python:3.11-slim

# Metadados
LABEL maintainer="Renan Kennedy <renan@exemplo.com>"
LABEL description="Sistema de monitoramento IoT com ESP8266 e DHT22"
LABEL version="1.0.0"

# Variáveis de ambiente
ENV PYTHONUNBUFFERED=1
ENV PYTHONDONTWRITEBYTECODE=1
ENV FLASK_APP=app.py
ENV FLASK_ENV=production

# Instalar dependências do sistema
RUN apt-get update && apt-get install -y \
    curl \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Criar usuário não-root
RUN groupadd -r appuser && useradd -r -g appuser appuser

# Definir diretório de trabalho
WORKDIR /app

# Copiar requirements primeiro (para cache de layers)
COPY requirements.txt .

# Instalar dependências Python
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Copiar código da aplicação
COPY . .

# Criar diretórios necessários
RUN mkdir -p /app/database /app/logs && \
    chown -R appuser:appuser /app

# Mudar para usuário não-root
USER appuser

# Expor porta
EXPOSE 5000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD curl -f http://localhost:5000/api/health || exit 1

# Comando de inicialização
CMD ["gunicorn", "--bind", "0.0.0.0:5000", "--workers", "4", "--timeout", "120", "--keep-alive", "2", "--max-requests", "1000", "--max-requests-jitter", "100", "app:app"]

