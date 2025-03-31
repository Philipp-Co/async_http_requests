from python:3.10

#COPY python_async_requests/ /python_async_requests/
#RUN cd /python_async_requests/ && ./build.sh && cp dist/asynchronous_http_requests-0.0.0.tar.gz /asynchronous_http_requests-0.0.0.tar.gz
#RUN pip install /asynchronous_http_requests-0.0.0.tar.gz

COPY system_test/server/requirements.txt /requirements.txt
RUN pip install -r /requirements.txt

COPY system_test/server/dummywebserver/ /dummywebserver/

WORKDIR /dummywebserver
ENV DJANGO_SETTINGS_MODULE=dummywebserver.settings
ENV http_proxy=""
ENV https_proxy=""
ENV HTTP_ROXY=""
ENV HTTPS_PROXY=""

CMD python manage.py makemigrations && \
    python manage.py migrate dummyapp && \
    gunicorn dummywebserver.wsgi --name=ahr_server --bind=0.0.0.0:8000 --workers=4
