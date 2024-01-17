/*
void receive_next()
{
    if (dfd.length() > 3 && dfd.substring(0, 3) != "TVC")
    {
      dfd = "";
      Serial.println("error");
    }
    else
    {
      if (dfd.substring((dfd.length() - 1), dfd.length()) == "?")
      {
        Serial.println(dfd.length());
        String command = dfd.substring(3, 6);
        Serial.println(command);
        String value = dfd.substring(6, dfd.length() - 1);
        Serial.println(command + " : " + value);
        Serial.println(value);
        if (command == "PAG")
        {
          Serial.println("puslapis");
        }
        dfd = "";
      }
    }
  }
  */