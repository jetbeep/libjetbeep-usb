#include "https_client.hpp"

#ifdef HTTP_CLIENT_NSURLSESSION
#import <Foundation/Foundation.h>
#include "../../io/iocontext_impl.hpp"

using namespace JetBeep;
using namespace std;

HttpsClient::HttpsClient() : m_log("https_client"), m_task(nullptr) {
  m_isCanceled.store(false);
  m_isPending.store(false);
};

HttpsClient::~HttpsClient() {
  m_isCanceled.store(true);
  m_isPending.store(false);

  NSURLSessionDataTask* oldTask = (NSURLSessionDataTask*)m_task;
  if (oldTask != nullptr) {
    [oldTask cancel];
    [oldTask release];
  }
}

Promise<Response> HttpsClient::request(RequestOptions& options) {
  if (m_isPending.load() == true) {
    throw runtime_error("previous request is not completed");
  }
  m_pendingRequest = Promise<Response>();
  m_options = options;
  NSURLSessionDataTask* oldTask = (NSURLSessionDataTask*)m_task;
  if (oldTask != nullptr) {
    [oldTask release];
    m_task = nullptr;
  }  

  @autoreleasepool {
    NSURLComponents *components = [[[NSURLComponents alloc] init] autorelease];    

    components.scheme = @"https";
    components.host = [[[NSString alloc] initWithUTF8String:options.host.c_str()] autorelease];
    components.path = [[[NSString alloc] initWithUTF8String:options.path.c_str()] autorelease];
    components.port = [[[NSNumber alloc] initWithInt:options.port] autorelease];    

    NSMutableArray *queryItems = [[[NSMutableArray alloc] init] autorelease];
    for (auto it = options.queryParams.begin(); it != options.queryParams.end(); it++) {
      NSString *key = [[[NSString alloc] initWithUTF8String:it->first.c_str()] autorelease];
      NSString *value = [[[NSString alloc] initWithUTF8String:it->second.c_str()] autorelease];
      NSURLQueryItem *item = [[[NSURLQueryItem alloc] initWithName:key value:value] autorelease];

      [queryItems addObject: item];
    }
    components.queryItems = queryItems;

    NSTimeInterval timeout = options.timeout;
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL: [components URL] cachePolicy: NSURLRequestUseProtocolCachePolicy timeoutInterval: timeout];

    switch (options.contentType) {
      case RequestContentType::JSON:
        [request addValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
        [request addValue:@"application/json" forHTTPHeaderField:@"Accept"];
      break;
      default:
      break;
    }

    switch (options.method) {
      case RequestMethod::GET:
        [request setHTTPMethod:@"GET"];
      break;
      case RequestMethod::POST:
        [request setHTTPMethod:@"POST"];
      case RequestMethod::PATCH:
        [request setHTTPMethod:@"PATCH"];
      break;
    }

    NSString *body = [NSString stringWithUTF8String:options.body.c_str()];
    [request setHTTPBody: [body dataUsingEncoding:NSUTF8StringEncoding]];

    for (auto it = options.headers.begin(); it != options.headers.end(); it++) {
      NSString *key = [NSString stringWithUTF8String:it->first.c_str()];
      NSString *value = [NSString stringWithUTF8String:it->second.c_str()];

      [request addValue:value forHTTPHeaderField:key];
    }

    NSURLSession* session = [NSURLSession sharedSession];    
    NSURLSessionDataTask* task = [session dataTaskWithRequest:request completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
      if (m_isCanceled.load() == true) {
        return;
      }
      @autoreleasepool {
        if (error) {
          const char* errorString = [[error localizedDescription] UTF8String];
          m_log.e() << errorString << Logger::endl;
          this->reject(make_exception_ptr(HttpErrors::NetworkError()));
          return;
        }

        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
        NSString *dataString = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        Response response;

        response.body = string([dataString UTF8String]);
        response.statusCode = [httpResponse statusCode];
        response.isHttpError = HttpsClient::isErrorStatusCode(response.statusCode);
        this->resolve(response);
      }
    }];
    m_task = [task retain];

    [task resume];
  }

  
  return m_pendingRequest;
}

void HttpsClient::reject(std::exception_ptr exception) {
  m_options.ioContext.m_impl->ioService.post([this, exception] {
     m_pendingRequest.reject(exception);
  });
}

void HttpsClient::resolve(Response response) {
  m_options.ioContext.m_impl->ioService.post([this, response] {
     m_pendingRequest.resolve(response);
  });
}

#endif