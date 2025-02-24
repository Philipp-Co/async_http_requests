from ctypes import cdll, byref, POINTER,pointer, c_void_p, c_char, CFUNCTYPE, c_char_p
from requests import get
from time import sleep, time_ns

ad = cdll.LoadLibrary('build/libahr.dylib')
ad.AHR_ResponseBody.argtypes = [c_void_p]
ad.AHR_ResponseBody.restype = c_char_p

start = None
end = None
done = 1

@CFUNCTYPE(c_void_p, c_void_p)
def callback(arg):
    global done
    #b = ad.AHR_ResponseBody(c_void_p(arg))    
    done = done + 1

ad.AHR_CreateProcessor.restype = POINTER(c_void_p)

a = ad.AHR_CreateProcessor()

#https://princekfrancis.medium.com/passing-a-callback-function-from-python-to-c-351ac944e041
#ad.AHR_ProcessorGet.argtypes = [c_void_p, POINTER(c_char), POINTER(func)]

ns = 20 * 1000**3
urls = [
    b'https://www.google.de',
    b'https://www.amazon.de',
    b'https://stackoverflow.com/questions/20147707/compiling-the-openssl-binary-statically',
    b'https://github.com/rockdaboot/libpsl',
    b'https://curl.se/libcurl/c/CURLOPT_URL.html',
    b'https://www.reddit.com/r/cpp_questions/comments/oo0w4v/libcurl_c_issues_retrieving_http_response_code/',
    b'https://www.tutorialspoint.com/c_standard_library/c_function_strchr.htm',
    b'https://embeddedartistry.com/blog/2017/07/05/printf-a-limited-number-of-characters-from-a-string/',
    b'https://mkyong.com/mac/where-does-homebrew-install-packages-on-mac/',
    b'https://cmake.org/cmake/help/latest/command/project.html#command:project',
    b'https://www.torsten-horn.de/techdocs/ascii.htm',
    b'https://openssl-library.org/source/',
    b'https://daniel.haxx.se/blog/2024/01/10/psl-in-curl/',
]
n = len(urls)

start = time_ns()
for i in range(0, n * 2):
    ad.AHR_ProcessorGet(a, urls[i % len(urls)], callback)

while True:
    if done == n*2:
        break
end = time_ns()

print(f'C Done: {done} in {(end - start) / 1000**3} -> {((end-start) / 1000**3) / len(urls)}')

pdone = 0
start = time_ns()
#while time_ns() - start < ns:
for i in range(0, n * 2):
    try:
        x = get(urls[i % len(urls)])#.content.decode("utf-8")
    except:
        pass
    pdone = pdone + 1
end = time_ns()

print(f'Py Done: {pdone} in {(end - start) / (1000**3)} -> {((end-start)/1000**3) / len(urls)}')

ad.AHR_DestroyProcessor(byref(a))
print(f'C/ende Done: {done}')
