from statemachine import StateMachine, State
from pyahr.async_http_requests import AHR_HttpRequestProcessor, AHR_Request, AHR_Response, AHR_DefaultEventHandler, AHR_HttpMethod, AHR_EventHandler
from abc import ABC, abstractmethod
from typing import List, Dict, Any, Optional
from logging import Logger
from dataclasses import dataclass
from typing_extensions import Self
from json import loads, dumps
from datetime import datetime

strftime_format: str = '%Y-%M-%DT%H:%M:%S.%z'

class ICounterService(ABC):
    
    @abstractmethod
    def create(self, name: str) -> Optional[int]:
        pass

    @abstractmethod
    def delete(self, name: str):
        pass
    
    @abstractmethod
    def number_of_available_increment_objects(self) -> int:
        pass

    @abstractmethod
    def request_increment(self) -> Optional[int]:
        pass
    
    @abstractmethod
    def free_increment(self, id: int):
        pass

    pass


class CountToNStatemachine(StateMachine):

    initial = State(initial=True)
    createing = State()
    counting = State()
    finished = State()
    cleaning_up = State()
    #error = State()

    start = initial.to(createing) | finished.to(finished)
    object_created = createing.to(counting)
    message_received = finished.to(finished) | counting.to(counting)
    object_deleted = cleaning_up.to(finished) | finished.to(finished)
    done = counting.to(cleaning_up) | finished.to(finished)

    def __init__(self, name: str, service: ICounterService, logger: Logger):
        self.__count: int = 10
        self.__service: ICounterService = service
        self.__events = []
        self.__number_of_increments_requested: int = 0
        self.__number_of_increment_responses: int = 0
        self.__name = name
        self.__logger: Logger = logger
        super().__init__()
        pass
   
    def append_event(self, event: str, message) -> Self:
        self.__events.append((event, message))
        return self

    def disptach_event(self) -> Self:
        if len(self.__events) > 0:
            event = self.__events.pop(0)
            self.send(event[0], message=event[1])
        return self

    def on_exit_initial(self):
        self.__logger.info('Exit initial')
        self.__service.create(self.__name)
        pass

    def on_enter_counting(self, event, message):
        if event == 'message_received':
            self.__number_of_increment_responses += 1

        while self.__number_of_increments_requested < self.__count:
            inkr: Optional[int] = self.__service.request_increment()
            if inkr is None:
                self.__logger.info('No Increment Object available.')
                break
            self.__logger.info('Inkrement requested...')
            self.__number_of_increments_requested += 1

        if self.__number_of_increments_requested >= self.__count-1 and self.__number_of_increment_responses == self.__number_of_increments_requested: 
            self.__logger.info(f'{self.__number_of_increments_requested} / {self.__count}')
            self.__service.delete(self.__name)
            self.append_event('done', {})
        pass
    
    def on_enter_finished(self):
        self.__logger.info('finished!')
        pass
    pass


class CountToN(AHR_EventHandler, ICounterService):

    def __init__(self, url: str, name: str, logger: Logger):
        super().__init__()
        self.__http_processor: AHR_HttpRequestProcessor = AHR_HttpRequestProcessor(
            url=url, event_handler=self, logger=logger
        )
        self.__ahr_put_request: AHR_Request = self.__http_processor.create_request().set_http_method(
            AHR_HttpMethod.PUT
        ).set_ressource(
            f'std/{name}/'
        )
        self.__http_processor.configure_request(
            self.__ahr_put_request
        )
        self.__ahr_delete_request: AHR_Request = self.__http_processor.create_request().set_http_method(
            AHR_HttpMethod.DELETE
        ).set_ressource(
            f'std/{name}/'
        )
        self.__http_processor.configure_request(
            self.__ahr_delete_request
        )
        self.__ahr_get_requests: Dict[int, AHR_Request] = {
            i.handle(): (
                False,
                i
            ) for i in [
                self.__http_processor.create_request().set_http_method(
                    AHR_HttpMethod.POST
                ).set_ressource(
                    f'std/{name}/'
                ) for j in range(0, 3) 
            ]
        }
        for item in self.__ahr_get_requests:
            print(f'Configure POST Request {item}')
            self.__http_processor.configure_request(
                self.__ahr_get_requests[item][1]
            )

        self.__state: CountToNStatemachine = CountToNStatemachine(
            name=name,
            service=self,
            logger=logger,
        )

        self.__logger: Logger = logger
        pass

    def start(self) -> Self:
        self.__state.append_event('start', message={})
        return self
    
    def disptach_event(self) -> Self:
        self.__state.disptach_event()
        return self

    def finished(self) -> bool:
        return self.__state.current_state == self.__state.finished

    def handle(self, response: AHR_Response) -> None:
        if response.request().handle() == self.__ahr_put_request.handle():
            self.__state.append_event(
                'object_created',
                message={
                        'object': response.body(),
                    }
            )
            return
        if response.request().handle() == self.__ahr_delete_request.handle():
            self.__state.append_event(
                'object_deleted',
                message={
                        'object': response.body(),
                    }
            )
            return

        self.__logger.info(f'Received Response {response.request().handle()}.')
        for item in self.__ahr_get_requests:
            if response.request().handle() == item:
                self.__state.append_event(
                    'message_received',
                    message={
                            'increment-id': item,
                            'object': response.body(),
                        }
                )
                self.__ahr_get_requests[item] = (
                    False,
                    self.__ahr_get_requests[item][1],
                )
                return
        self.__logger.info(f'Unable to find Request Object for Response.')
        pass

    def create(self, name: str) -> bool:
        #self.__http_processor.configure_request(self.__ahr_put_request)
        self.__ahr_put_request.set_body(
            dumps(
                {
                    'datetime': datetime.now().strftime(strftime_format),
                }
            )
        )
        self.__http_processor.configure_request(
            self.__ahr_put_request
        )
        self.__http_processor.make_request(self.__ahr_put_request)
        #self.handle(0)
        return True
    
    def delete(self, name: str):
        self.__http_processor.make_request(self.__ahr_delete_request)
        pass
    
    def request_increment(self) -> Optional[int]:
        #for i in range(0, len(self.__inrement_objects)):
        #    if not self.__inrement_objects[i]:
        #        self.__inrement_objects[i] = True
        #        return i
        for item in self.__ahr_get_requests:
            if not self.__ahr_get_requests[item][0]:
                self.__ahr_get_requests[item] = (
                    True,
                    self.__ahr_get_requests[item][1]
                )
                self.__ahr_get_requests[item][1].set_body(
                    dumps(
                        {
                            'datetime': datetime.now().strftime(strftime_format),
                        }
                    )
                )
                self.__http_processor.configure_request(
                    self.__ahr_get_requests[item][1]
                )
                self.__http_processor.make_request(
                    self.__ahr_get_requests[item][1]
                )
                return item
        return None
    
    def free_increment(self, id: int):
        #self.__inrement_objects[id] = False
        self.__ahr_get_requests[id] = (
            False,
            self.__ahr_get_requests[id][1]
        )
        pass

    def number_of_available_increment_objects(self) -> int:
        #count = 0
        #for i in range(0, len(self.__inrement_objects)):
        #if not self.__inrement_objects[i]:
        #        count += 1
        #return count
        count = 0
        for item in self.__ahr_get_requests:
            if not self.__ahr_get_requests[item][0]:
                count += 1
        return count
    pass

