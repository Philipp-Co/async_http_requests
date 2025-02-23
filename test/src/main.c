
#include <async_http_requests/async_http_requests.h>
#include <async_http_requests/request_processor.h>

#include <unistd.h>
#include <stdio.h>

static int done = 0;

static void Callback(AHR_HttpResponse_t response)
{
    printf("--------------------------------------------------------------\n");
    printf("%s\n", AHR_ResponseBody(response));
    printf("--------------------------------------------------------------\n");
    done++;
}

int main(int argc, char **argv)
{
    AHR_Processor_t processor = AHR_CreateProcessor();
    
    AHR_ProcessorGet(processor, "www.google.de", Callback);
    AHR_ProcessorGet(processor, "www.amazon.de", Callback);
    AHR_ProcessorGet(processor, "www.web.de", Callback);
    AHR_ProcessorGet(processor, "www.ppilihpllork.de", Callback);

    do
    {
        printf("main...\n");
        sleep(1);
    } while(done != 4);

    AHR_DestroyProcessor(&processor);
    return 0;
}
