# Docker Setup

This document provides instructions for building and running memoraxx using Docker.

## Prerequisites

- Docker installed on your system
- Ollama running on the host (or in a separate container)

## Building the Image

### Local Build

```bash
docker build -t memoraxx .
```

This creates a Docker image based on Ubuntu 22.04 with all necessary dependencies installed and the application built.

### Pull from GitHub Container Registry

Pre-built images are available on GHCR:

```bash
docker pull ghcr.io/bniladridas/memoraxx:latest
docker run -it ghcr.io/bniladridas/memoraxx:latest
```

Images are automatically built and pushed on every push to the `main` branch.

## Running the Container

```bash
docker run -it memoraxx
```

## Networking with Ollama

Since memoraxx connects to Ollama on `localhost:11434`, you have two options:

### Option 1: Ollama on Host

Run Ollama on your host machine:
```bash
ollama serve
ollama pull llama3.2
```

Then run the container with host networking:
```bash
docker run -it --network host memoraxx
```

### Option 2: Ollama in Docker

Run Ollama in a separate container:
```bash
docker run -d --name ollama ollama/ollama
docker exec ollama ollama pull llama3.2
```

Then link the containers:
```bash
docker run -it --link ollama:ollama memoraxx
```
Set the Ollama URL to `http://ollama:11434` (requires code modification or environment variable support).

## Dockerfile Details

The Dockerfile uses Ubuntu 22.04 and installs:
- build-essential
- cmake
- libcurl4-openssl-dev
- nlohmann-json3-dev

It copies the source code, builds with CMake, and sets the default command to run the executable.

## .dockerignore

The `.dockerignore` file excludes:
- build/ directory
- *.log files
- .git/ directory
- Documentation files
- CI configuration

This reduces the build context size.