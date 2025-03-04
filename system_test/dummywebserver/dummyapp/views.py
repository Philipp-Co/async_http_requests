from django.shortcuts import render
from rest_framework.views import APIView
from rest_framework.response import Response
from logging import getLogger, Logger
from http import HTTPStatus


class GenericView(APIView):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__logger: Logger = getLogger(self.__class__.__name__)

    def get(self, request):
        self.__logger.info('GET called.') 
        self.__logger.info(f'Query Parameter: {request.GET}')
        self.__logger.info(f'Header: {request.headers}')
        return Response(status=HTTPStatus.OK)

    def post(self, request):
        self.__logger.info('POST called.') 
        self.__logger.info(f'Query Parameter: {request.query_params}')
        self.__logger.info(f'Body: {request.body.decode()}')
        return Response(status=HTTPStatus.OK)

    def put(self, request):
        self.__logger.info('PUT called.') 
        self.__logger.info(f'Query Parameter: {request.PUT}')
        self.__logger.info(f'Body: {request.content}')
        return Response(status=HTTPStatus.OK)
    
    def delete(self, request):
        self.__logger.info('DELETE called.') 
        self.__logger.info(dir(request))
        return Response(status=HTTPStatus.OK)

    pass
