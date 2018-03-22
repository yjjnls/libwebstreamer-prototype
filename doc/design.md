# Overview

libwebstreamer is plugin (dynamic library) to manipulate media stream.

this plugin is compoosed of compents which provice functions live, push to talk ... 

```
             +------------------------+
             |                        |
stream => [Endpoint]               [Endpoint]--channel-> VLC     
             |         COMPONENT      |
             |                 [        ]--channel-->|
          [Endpoint]           [Endpoint]            |WebBrowser|
             |                 [        ]<--channel--|  
             +------------------------+
```

* Processor 

  top level element, which includes one or more Endpoints. internal this is corresponding pipeline of GStreamer

* Endpoint

  Endpoint is entity for Component to fetch or provide stream with outside module/device,which includes one or more Channels.

  ```javascript

  Endpoint = {
    name       : 'string'
    url        : 'string'
    initiative : 'bool'       //for WebRTC the initiative=true mean offer sender
    channel    : '[Channel]'; // type is array of Channel
  }
  ```

* Channel
  Channel is the real entity to send or receiv streamer, that is Channel is one-way element(in or out in Side of Componet internal view)

  ```javascript
  Channel ={
    name       : 'string'
    codec      : 'string' //media codec format 
                          // H264/H265/VP8
                          // G711A/OPUS
    direction  : 'string' // stream direction of the channel from component inter view.
                          // 'in' or 'out'                         
  }
  ```
## Common functionality 

```javascript
   
```

### 1. Create processor
  
  * input param

    ```javascript
       {
         name : 'string' // name of the Processor
         type : 'string' // type of the Processor
                         // string enum{livestream,PTT}        
       }
    ```
    name unique is guaranteed by invoker

  * output
    success or falure

### 2. Destroy processor
  * input param
    ```javascript
       {
         name : 'string' // name of the Processor
       }
    ```
  * output
    success or falure

## livestream

There is one performer type Endpoint and manay audience type Enpoints. The performer (Endpoint) stream will distribute to audience(Endpoints),and one of the audience audio stream may send back to performer.

```javascript

   livestream = webstreamer.Create('livestream')
   enpoint.name = 'performer'
   livestream.AddPerformer( endpoint)
   enpoint.name = 'tom'
   enpoint.channel[0].name='callback-chnnel'
   livestream.AddAudience( endpoint )
   ....
   livestream.AddAudience( endpoint )

   livestream.SetAudioCallback(endpoint='tom',channel='callback-channel') //set tom as the audio call back one

   livestream.SetAduioCallback(null);//no audio call back

   livestream.Remove(endpoint='tom')

   webstreamer.Destroy()

```
## 1.add performer

add a performer type endpoint

```javascript 
   {
     type : 'string' //=performer
     endpoint : 'Endpoint' //endpoint of the performer
   }
```
## 1. add audience

add a performer type endpoint

```javascript 
   {
     type : 'string' //=audience
     endpoint : 'Endpoint' //endpoint of the performer
   }
```


## Promise

in the libwebstreamer, each function has two result success or faulure.

success coresspoding Javascript promise resolve, the detail content is infered bay the call function. failure will be always an json object
```javascript
   {
     name : 'string' 
     message: 'string' 
   }
