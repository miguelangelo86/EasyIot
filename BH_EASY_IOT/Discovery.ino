
fauxmoESP fauxmo;

void startAlexaDiscovery()
{
  fauxmo.createServer(false);
  fauxmo.setPort(80); // required for gen3 devices
  fauxmo.enable(true);
  fauxmo.onSetState([](unsigned char device_id, const char *device_name, bool state, unsigned char value) {
    JsonArray &_devices = getStoredSwitchs();
    for (int i = 0; i < _devices.size(); i++)
    {
      JsonObject &switchJson = _devices[i];
      if (switchJson.get<String>("name").equals(String(device_name)))
      {
        stateSwitch(switchJson, state ? "ON" : "OFF");
        break;
      }
    }
  });
}

void reloadAlexaDiscoveryServices()
{
  JsonArray &_devices = getStoredSwitchs();
  for (int i = 0; i < _devices.size(); i++)
  {
    JsonObject &switchJson = _devices[i];
    String _name = switchJson.get<String>("name");
    fauxmo.removeDevice(_name.c_str());
    fauxmo.addDevice(_name.c_str());
  }
}

void  rebuildDiscoverySwitchMqttTopics(JsonObject &switchJson)
{
  String ipMqtt = getConfigJson().get<String>("mqttIpDns");
  if (ipMqtt == "")
    return;
  String prefix = getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix");
    String _id = switchJson.get<String>("id");
    String _name = switchJson.get<String>("name");
    String type = switchJson.get<String>("type");
    if (type.equals("cover"))
    {
      publishOnMqtt(prefix + "/cover/" +  _id + "/config", createHaCover(switchJson), true);
      
    
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    else if (type.equals("light"))
    {
      publishOnMqtt(prefix + "/light/" +  _id + "/config", createHaLight(switchJson), true);
      
      
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    else if (type.equals("switch"))
    {
      publishOnMqtt(prefix + "/switch/" +  _id + "/config", createHaSwitch(switchJson), true);
     
      
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    else if (type.equals("lock"))
    {
      publishOnMqtt(prefix + "/lock/" +  _id + "/config", createHaLock(switchJson), true);
      
       
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    
    logger("[MQTT] RELOAD MQTT SWITCH DISCOVERY OK");
}
void  rebuildDiscoverySensorMqttTopics( JsonObject &sensorJson )
{  
   String ipMqtt = getConfigJson().get<String>("mqttIpDns");
   if (ipMqtt == "")
    return;
  String prefix = getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix");
    String _class = sensorJson.get<String>("class");
    JsonArray &functions = sensorJson.get<JsonVariant>("functions");
    for (int i = 0; i < functions.size(); i++)
    {
      JsonObject &f = functions.get<JsonVariant>(i);
      String _id =  String(ESP.getChipId())+normalize(f.get<String>("name"));
      if (_class.equals("binary_sensor"))
      {
        publishOnMqtt(prefix + "/binary_sensor/" + _id + "/config", createHaBinarySensor(sensorJson), true);
  
      }
      else if (_class.equals("sensor"))
      { 
        
        publishOnMqtt(prefix + "/sensor/" +  _id + "/config", createHaSensor(sensorJson, f), true);
        
        
      
      }
    }
      logger("[MQTT] RELOAD MQTT SENSOR DISCOVERY OK");
  }


void loopDiscovery()
{
  fauxmo.handle();
}

String createHaSensor(JsonObject &_sensorJson, JsonObject &_f)
{
  String object = "";
  const size_t capacity = JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& sensorJson = jsonBuffer.createObject();
  sensorJson.set("name", _f.get<String>("name"));
  sensorJson.set("state_topic", _sensorJson.get<String>("mqttStateTopic"));
  sensorJson.set("unit_of_measurement", _f.get<String>("unit"));
  sensorJson.set("availability_topic", getAvailableTopic());
  sensorJson.set("device_class", _f.get<String>("uniqueName"));
  sensorJson.set("value_template", "{{ value_json."+_f.get<String>("uniqueName")+"}}");
 
  sensorJson.printTo(object);
  return object;
}
String createHaBinarySensor(JsonObject &_sensorJson)
{
  String object = "";
  const size_t capacity = JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& sensorJson = jsonBuffer.createObject();
  sensorJson.set("name", _sensorJson.get<String>("name"));
  sensorJson.set("state_topic", _sensorJson.get<String>("mqttStateTopic"));
  sensorJson.set("payload_on", String(PAYLOAD_ON));
  sensorJson.set("payload_off", String(PAYLOAD_OFF));
  sensorJson.set("availability_topic", getAvailableTopic());
  sensorJson.set("device_class", "opening");
  sensorJson.printTo(object);
  return object;
}
String createHaLock(JsonObject &_switchJson)
{
  String object = "";
   const size_t capacity = JSON_OBJECT_SIZE(7);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& switchJson = jsonBuffer.createObject();
  switchJson.set("name", _switchJson.get<String>("name"));
  switchJson.set("command_topic", _switchJson.get<String>("mqttCommandTopic"));
  switchJson.set("state_topic", _switchJson.get<String>("mqttStateTopic"));
  switchJson.set("retain", _switchJson.get<bool>("retain"));
  switchJson.set("availability_topic", getAvailableTopic());
  switchJson.set("payload_lock", String(PAYLOAD_LOCK));
  switchJson.set("payload_unlock", String(PAYLOAD_UNLOCK));
  switchJson.printTo(object);
  return object;
}
String createHaSwitch(JsonObject &_switchJson)
{
  String object = "";
  const size_t capacity = JSON_OBJECT_SIZE(7);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& switchJson = jsonBuffer.createObject();
  switchJson.set("name", _switchJson.get<String>("name"));
  switchJson.set("command_topic", _switchJson.get<String>("mqttCommandTopic"));
  switchJson.set("state_topic", _switchJson.get<String>("mqttStateTopic"));
  switchJson.set("retain", _switchJson.get<bool>("retain"));
  switchJson.set("payload_on", String(PAYLOAD_ON));
  switchJson.set("availability_topic", getAvailableTopic());
  switchJson.set("payload_off", String(PAYLOAD_OFF));
  switchJson.printTo(object);
  return object;
}
String createHaLight(JsonObject &_switchJson)
{
  String object = "";
  const size_t capacity = JSON_OBJECT_SIZE(7);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& switchJson = jsonBuffer.createObject();
  switchJson.set("name", _switchJson.get<String>("name"));
  switchJson.set("command_topic", _switchJson.get<String>("mqttCommandTopic"));
  switchJson.set("state_topic", _switchJson.get<String>("mqttStateTopic"));
  switchJson.set("retain", _switchJson.get<bool>("retain"));
  switchJson.set("availability_topic", getAvailableTopic());
  switchJson.set("payload_on", String(PAYLOAD_ON));
  switchJson.set("payload_off", String(PAYLOAD_OFF));
  switchJson.printTo(object);
  return object;
}
String createHaCover(JsonObject &_switchJson)
{
  String object = "";
  const size_t capacity = JSON_OBJECT_SIZE(10);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& switchJson = jsonBuffer.createObject();
  switchJson.set("name", _switchJson.get<String>("name"));
  switchJson.set("command_topic", _switchJson.get<String>("mqttCommandTopic"));
  switchJson.set("position_topic", _switchJson.get<String>("mqttPositionStateTopic"));
  switchJson.set("retain", _switchJson.get<bool>("retain"));
  switchJson.set("position_open", 100);
  switchJson.set("availability_topic", getAvailableTopic());
  switchJson.set("position_closed", 0);
  switchJson.set("payload_open", String(PAYLOAD_OPEN));
  switchJson.set("payload_close", String(PAYLOAD_CLOSE));
  switchJson.set("payload_stop", String(PAYLOAD_STOP));
  switchJson.printTo(object);
  return object;
}
void reloadMqttSubscriptionsAndDiscovery(){
  
  String ipMqtt = getConfigJson().get<String>("mqttIpDns");
  if (ipMqtt == "")
    return;
   String prefix = getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix");
    JsonArray &switches = getStoredSwitchs();
  for (int i = 0; i < switches.size(); i++)
  {
    JsonObject &switchJson = switches.get<JsonVariant>(i);
     
    String type = switchJson.get<String>("type");
    if (type.equals("cover"))
    {  
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    else if (type.equals("light"))
    {
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    else if (type.equals("switch"))
    {
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    else if (type.equals("lock"))
    { 
      subscribeOnMqtt(switchJson.get<String>("mqttCommandTopic"));
    }
    rebuildDiscoverySwitchMqttTopics(switchJson);
  }
    JsonArray &sensors = getStoredSensors();
  for (int i = 0; i < sensors.size(); i++)
  {
    JsonObject& sensorJson = sensors.get<JsonVariant>(i);
     rebuildDiscoverySensorMqttTopics(sensorJson);
  }
  
    logger("[MQTT] RELOAD MQTT SUBSCRIPTIONS OK");
}


void removeFromAlexaDiscovery(String _name)
{
  fauxmo.removeDevice(_name.c_str());
}

void removeFromHaDiscovery(String type, String _id)
{
  publishOnMqtt(getConfigJson().get<String>("homeAssistantAutoDiscoveryPrefix") + "/" + type +"/" + _id + "/config", "", false);
}
