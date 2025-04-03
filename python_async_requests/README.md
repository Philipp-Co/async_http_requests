# python_async_http_requests - pyahr

This Package contains the Source Code for asynchronous Http Requests.

pyahr implements a Wrapper around libahr.
pyahr provides an easy Interface to interact with libahr from within python.


    from async_http_requests import AHR_HttpRequestProcessor
    
    processor: AHR_HttpRequestProcessor = AHR_HttpRequestProcessor()
    request: AHR_Request = processor.create_request()
    processor.configure_request(
        request.set_http_method(
            HttpMethod.POST
        ).set_body(
            '{"test": "x"}'
        )
    ).make_request(
        request
    )


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

