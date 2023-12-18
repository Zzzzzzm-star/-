// index.js
//获取应用实例
const app =getApp()

const { connect } = require('../../utils/mqtt')

const mqttHost = 'broker.emqx.io'// mqtt服务器域名
const mqttPort = '8084' // mqtt服务器端口


const deviceSubTopic = '/mysmarthome/sub' // 设备订阅 （小程序发布命令的Topic )
const devicePubTopic = '/mysmarthome/pub' // 设备发布 （小程序订阅数据的Topic )

const mpSubTopic = devicePubTopic
const mpPubTopic = deviceSubTopic

Page({
  data: {
    client: null,
    Temp:0,
    Hum:0,
    Led:false,
    Beep:false
  },
  onLedChange(event){
    const that = this
    console.log(event.detail.value);
    const sw = event.detail.value
    that.setData({Led:sw})

    if(sw){
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"LED",
        value:1
      }),function (err){
        if(!err){
          console.log('成功下发一个指令——开灯')
        }
      })

    }else{
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"LED",
        value:0
      }),function (err){
        if(!err){
          console.log('成功下发一个指令——关灯')
        }
      })

    }

  },
  onBeepChange(event){
    const that = this
    console.log(event.detail.value);
    const sw = event.detail.value
    that.setData({Beep:sw})

    if(sw){
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"BEEP",
        value:1
      }),function (err){
        if(!err){
          console.log('成功下发一个指令——打开报警器')
        }
      })

    }else{
      that.data.client.publish(mpPubTopic,JSON.stringify({
        target:"BEEP",
        value:0
      }),function (err){
        if(!err){
          console.log('成功下发一个指令——关闭报警器')
        }
      })

    }

  },

  //事件处理函数
  onShow() {
    const that = this
    that.setData({
      //wxs 实际上就是wss => wss 实际上就是拥有SSL加密通信的websockect协议
      client: connect(`wxs://${mqttHost}:${mqttPort}/mqtt`)
    })
    that.data.client.on('connect',function() {
      console.log('成功连接上MQTT服务器! ')
      wx.showToast({
        title: '连接成功',
        icon:'success',
        mask: true
      })

        that.data.client.subscribe(mpSubTopic,function(err) {
          if(!err){
            console.log('成功订阅设备上行数据Topic!')
          }
        })


    })
    
    that.data.client.on('message',function(topic,message){
       console.log(topic);
      // console.log(message);
      //message 是十六进制的buffer字节流

      let dataFromDev = {}
      try{
          dataFromDev = JSON.parse(message)
         console.log(dataFromDev);
        that.setData({
          Temp:dataFromDev.Temp,
          Hum:dataFromDev.Hum,
          Led:dataFromDev.Led,
          Beep:dataFromDev.Beep,
        })



      }catch (error) {
      //  console.log(error);
        console.log('JSON解析失败',error)

      }




    })


  },
})