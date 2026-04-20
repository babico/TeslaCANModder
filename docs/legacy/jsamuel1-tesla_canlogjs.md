---
title: jsamuel1-tesla_canlogjs
description: A full-stack Tesla CAN logging pipeline - captures CAN messages on a Raspberry Pi 4 using a comma.ai White Panda, stores 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: jsamuel1
repo: tesla_canlogjs
---

# jsamuel1-tesla_canlogjs

## Overview

A full-stack Tesla CAN logging pipeline: captures CAN messages on a Raspberry Pi 4 using a comma.ai White Panda, stores them as CSV, uploads to AWS S3, then processes them via an AWS Lambda + Timestream pipeline using the Model3CAN.dbc file to decode signals. Includes AWS CDK infrastructure-as-code for cloud deployment.

## Technical Details

- **Platform**: Raspberry Pi 4 (capture) + AWS Cloud (processing/storage)
- **Language**: Python (capture/upload), Python + AWS CDK (cloud processing)
- **CAN Interface**: comma.ai White Panda (USB CAN adapter via `panda` Python library)
- **License**: MIT License (Copyright 2020 Josh Samuel)

## Architecture

- `rpi/can_capture.py` — Captures CAN frames from the Panda adapter, writes hourly CSV files with bus, message ID, data, length, and timestamp
- `rpi/can_upload.py` — Monitors for completed CSV files, compresses with gzip, and uploads to AWS S3
- `rpi/install.sh` — Sets up systemd services for auto-capture and upload
- `processing/canmsgtosignals.py` — AWS Lambda handler that reads CSVs from S3, decodes CAN messages using `cantools` + `Model3CAN.dbc`, and writes decoded signals to AWS Timestream
- `processing/Model3CAN.dbc` — Copy of Josh Wardell's Model 3 DBC file
- `cdk/` — AWS CDK infrastructure for S3, Lambda, Timestream, Route53

## CAN Bus Integration

Captures raw CAN frames from all three buses (Bus 0, 1, 2) via the comma.ai Panda interface. The capture script logs every frame with bus ID, hex message ID, hex data payload, DLC, and ISO 8601 timestamp. The processing pipeline decodes these using the Model3CAN.dbc definitions via the `cantools` library.

## Relevance to Our Project

Demonstrates a complete capture-to-cloud pipeline for Tesla CAN data. The capture script and DBC-based processing approach are good references.

- **Reusability**: Medium
- **Key Takeaways**:
  - Clean CAN capture script using comma.ai Panda library
  - DBC-based signal decoding via `cantools` Python library
  - Example of cloud-based CAN data processing pipeline (S3 → Lambda → Timestream)
  - Hourly CSV rotation pattern useful for long-term in-vehicle logging
