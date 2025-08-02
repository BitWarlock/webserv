# Docker guide - How to run the project?

## 1- Build the Docker Image

```bash
docker build -t webserv-ssh .
```

## 2- Run the Container

```bash
docker run -d \
  -p 2222:22 \
  -p 8003:8003 \
  -v $(pwd):/project \
  --name webserv-container \
  webserv-ssh
```

> ✅ The `-v $(pwd):/project` syncs your local directory with the container, so any changes are reflected **immediately**—no need to rebuild the image!

## Access the Running Container

```bash
docker exec -it webserv-container bash
```

## Access Your Web Server

Open your browser and go to:

```
http://localhost:8003
```

Your web server should be accessible there normally.

---

##  Stop and Remove the Container

```bash
docker rm -f webserv-container
```
