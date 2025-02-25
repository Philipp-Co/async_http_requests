# Asynchronous Http Requests

This Package contains the Source Code for asynchronous Http Requests.

# How to use

Set the LIB_AHR_PATH Environmentvariable to configure the Path to the AHR Library.

    export LIB_AHR_PATH=.../ahr.dylib

# Build 

Note: This Package depends on the libahr. The Versions MUST match. 

    python -m build --sdist

# Test

Create venv, install Requirements and Test.

    python -m venv .venv
    source .venv/bin/activate
    pip install -r requirements.txt

Run Test for the Package.

    python test.py

