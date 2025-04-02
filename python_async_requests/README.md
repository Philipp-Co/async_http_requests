# Asynchronous Http Requests

This Package contains the Source Code for asynchronous Http Requests.

# Build Package 

This Project builds libahr.so/.dylib when installing it into the Projectenvironment.

    python -m venv .venv && source .venv/bin/activate
    pip install build
    python -m build --sdist

Now dist/ should contain the Package ahr-x.y.z.tar.gz, which is the installable Pythonpackage.

# Setup Package

Setup Project, create a virtual Environment.

    python -m venv .venv
    source .venv/bin/activate

Install AHR

    pip install ahr-x.y.z.tar.gz 

Now .venv/lib/site-packages/ should contain the shared Library "ahr-x.y.z.so".
Set the LIB_AHR_PATH Environmentvariable to configure the Path to the AHR Library.

    export LIB_AHR_PATH=path/to/project/.venv/lib/site-packages/ahr-x.y.z.so

Note that the Fileextension .so denpends on the Operating System the Library is build on.
On Macos it may be ".dylib" and on Linux based Systems it may be ".so".

# Test Package

## Unittest

Run Unittests.

    cd test/
    python run_test

## Systemtest

Run Systemtests. These Tests use a simple Webserver to check if this Package works with a "real" Webserver.

### Docker Compose

To run this kind of Test install docker-compose. 
The Repo contains a docker-compose.yaml file which defines a Client and a Server Service.
The Client Service contains the Testcode, the Server Service is used as its Counter-Part.

