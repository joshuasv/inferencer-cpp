# Inferencer-CPP

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C++-68.4%25-green.svg)
![Python](https://img.shields.io/badge/Python-13.9%25-blue.svg)
![Docker](https://img.shields.io/badge/Docker-10.6%25-blue.svg)
![CMake](https://img.shields.io/badge/CMake-6.8%25-green.svg)

## Overview

Inferencer-CPP is a simple C++ application for object detection and tracking using the Qt framework. It leverages modern C++ features and integrates with various models to provide efficient and accurate inferencing capabilities.

## Features

- **Object Detection**: Detect objects in images using pre-trained models.
- **Tracking**: Track detected objects across frames.
- **Qt Integration**: User-friendly interface using Qt.
- **Docker Support**: Easily deployable with Docker.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Folder structure](#folder-structure)
- [Contributing](#contributing)
- [License](#license)

## Installation

### Prerequisites
- Docker and Docker Compose

### Steps

1. **Allow local root user to access the X server:**
   ```bash
    # This is necessary for running graphical applications as root.
    # Use with caution as it can pose a security risk if not managed properly.
    # https://unix.stackexchange.com/questions/330366/how-can-i-run-a-graphical-application-in-a-container-under-wayland
    xhost +SI:localuser:root
    # Check that it was added
    xhost
   ```

1. **Set environment variables:**
   ```bash
   # Check your video devices
   ls /dev/video*
   # Install v4l2 tool
   sudo apt-get install v4l-utils
   # Identify the webcam you want to use
   v4l2-ctl --device=/dev/video<CAM_ID> --info
   # Update .env file CAM_ID varialbe with your webcam id
   # Update the MODEL_FPATH variable in the .env file with the path to the desired model.
   ```
1. **Clone the repository:**
   ```bash
   git clone https://github.com/joshuasv/inferencer-cpp.git
   cd inferencer-cpp
   ```

2. **Run application:**
   ```bash
   docker compose run business_logic
   # This step may take some time when run for the first time
   ```

3. **Shutting down all services:**
   ```bash
   docker compose down
   ```

## Usage

### Inference application

- Press 's' to start inference
- Press 'q' to quit the application 

### Command line interface (CLI) application

Select one of the options by typing its number and press Enter. Options are:

1. This option allows you to process the next order in the queue.
2. This option allows you to fill a specific order by entering its unique ID.
3. This option allows you to exit the CLI tool.

### API

Access the API through the web browser at `http://0.0.0.0:5000/` and generate some mock data.

## Folder Structure

- `src`: Source code files.
- `models`: Pre-trained models.
- `scripts`: Utility scripts.
- `external`: External dependencies.
- `tools`: Additional tools.
- `poc-specific`: Proof-of-concept specific applications

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any features, bug fixes, or enhancements.

1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/your-feature`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin feature/your-feature`).
5. Open a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details.