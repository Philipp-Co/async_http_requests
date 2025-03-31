from django.shortcuts import render
from rest_framework.views import APIView
from rest_framework.serializers import Serializer, DateTimeField
from rest_framework.response import Response
from logging import getLogger, Logger
from http import HTTPStatus
from django.utils import timezone
from dummyapp.models import Dummy
from rest_framework.request import Request


class GetResponse(Serializer):

    datetime = DateTimeField()

    pass


class SimpleGet(APIView):

    def get(self, request):
        return Response()


class GenericView(APIView):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__logger: Logger = getLogger(self.__class__.__name__)

    def get(self, request, name: str):
        self.__logger.info('GET called.') 
        self.__logger.info(f'Query Parameter: {request.GET}')
        self.__logger.info(f'Header: {request.headers}')
        try:
            return Response(
                status=HTTPStatus.OK,
                data={
                    'datetime': timezone.now().strftime('%Y-%m-%dT%H:%M:%S.%f%z'),
                    'object': {
                        'value': Dummy.objects.get(name=name).value
                    },
                },
            )
        except Exception as e:
            self.__logger.exception(e)
            return Response(status=HTTPStatus.INTERNAL_SERVER_ERROR)

    def post(self, request, name: str):
        self.__logger.info('POST called.') 
        self.__logger.info(f'Query Parameter: {request.query_params}')
        self.__logger.info(f'Body: {request.body.decode()}')
        
        try:
            obj: Dummy = Dummy.objects.get(name=name)
            obj.value += 1
            obj.save()
        except Exception as e:
            self.__logger.exception(e)
            return Response(status=HTTPStatus.INTERNAL_SERVER_ERROR)
        return Response(status=HTTPStatus.OK)

    def put(self, request, name: str):
        self.__logger.info('PUT called.') 
        self.__logger.info(f'Query Parameter: {request.query_params}')
        self.__logger.info(f'Body: {request.body}')
        try:
            obj: Dummy = Dummy(name=name, value=0)
            obj.save()
        except Exception as e:
            self.__logger.exception(e)
            return Response(status=HTTPStatus.INTERNAL_SERVER_ERROR)
        return Response(
            status=HTTPStatus.OK,
            data={
                'datetime': timezone.now().strftime('%Y-%m-%dT%H:%M:%S.%f%z')
            }
        )
    
    def delete(self, request, name: str):
        self.__logger.info('DELETE called.') 
        self.__logger.info(dir(request))
        try:
            Dummy.objects.get(name=name).delete()
        except Exception as e:
            self.__logger.exception(e)
            return Response(status=HTTPStatus.INTERNAL_SERVER_ERROR)
        return Response(status=HTTPStatus.OK)

    pass
