class c_key
{
private:
  unsigned char pin = 0;
public:
  c_key(unsigned char pin = 0)
  {
    this->setup(pin);
  }

  ~c_key()
  {
    
  }

  void setup(unsigned char pin)
  {
    this->pin = pin;
  }

  bool down = false;
  bool clicked_last_update = false;
  bool released_last_update = false;
  
  void update()
  {              
    this->clicked_last_update = false;
    this->released_last_update = false;

    if (!this->pin)
    {
      return;
    }
    
    if (digitalRead(this->pin) == LOW)
    {
      if (!this->down)
      {
        this->clicked_last_update = true;
      }

      this->down = true;
    }
    else
    {
      if (this->down)
      {
        this->released_last_update = true;
      }
      
      this->down = false;
    }
  }
};

class c_color
{
public:
  c_color(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255)
  {
    this->set(r, g, b);
  }

  ~c_color()
  {
    
  }

  void set(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255)
  {
    this->r = r;
    this->g = g;
    this->b = b;
  }

  static c_color from_hsb( float hue, float saturation, float brightness)
  {
    float h = hue == 1.f ? 0 : hue * 6.f;
    float f = h - (int)h;
    float p = brightness * ( 1.f - saturation );
    float q = brightness * ( 1.f - saturation * f );
    float t = brightness * ( 1.f - ( saturation * ( 1.f - f ) ) );
  
    if( h < 1 )
    {
      return c_color(brightness * 255, t * 255, p * 255);
    }
    else if( h < 2 )
    {
      return c_color(q * 255, brightness * 255, p * 255);
    }
    else if( h < 3 )
    {
      return c_color(p * 255, brightness * 255, t * 255);
    }
    else if( h < 4 )
    {
      return c_color(p * 255, q * 255, brightness * 255);
    }
    else if( h < 5 )
    {
      return c_color(t * 255, p * 255, brightness * 255);
    }
    
    return c_color(brightness * 255, p * 255, q * 255);
  }

  unsigned char r = 255;
  unsigned char g = 255;
  unsigned char b = 255;  
};

class c_led
{
public:
  c_led(unsigned char pin_r = 0, unsigned char pin_g = 0, unsigned char pin_b = 0)
  {
    this->setup(pin_r, pin_g, pin_b);
  }

  ~c_led()
  {
    
  }

  void setup(unsigned char pin_r = 0, unsigned char pin_g = 0, unsigned char pin_b = 0)
  {
    this->pin_r = pin_r;
    this->pin_g = pin_g;
    this->pin_b = pin_b;
  }

  void turn_on(c_color color)
  {
    analogWrite(this->pin_r, 255 - color.r);
    analogWrite(this->pin_g, 255 - color.g);
    analogWrite(this->pin_b, 255 - color.b);
  }

  void turn_off()
  {
    analogWrite(this->pin_r, 255);
    analogWrite(this->pin_g, 255);
    analogWrite(this->pin_b, 255);
  }
  
  void update()
  {
    if (!this->active)
    {
      this->turn_off();
      return;
    }

    if (!this->pin_r && !this->pin_g && !this->pin_b)
    {
      return;
    }

    switch(this->reactive)
    {
    case 0:
      break;
    case 1:
      if (!this->should_react)
      {
        this->turn_off();
        return;
      }
      
      break;
    case 2:
      if (this->should_react)
      {
        this->should_react = false;
        this->turn_off();
        return;
      }
      break;
    }

    switch(this->colormode)
    {
    case 0:
      this->turn_on(this->color);
      break;
    case 1:
      this->turn_on(c_color::from_hsb(this->hue, 1.f, 1.f));
      this->hue += this->hue_add_fast;
      break;
    case 2:
      this->turn_on(c_color::from_hsb(this->hue, 1.f, 1.f));
      this->hue += this->hue_add_slow;
      break;
    case 3:
      this->turn_on(c_color::from_hsb(this->hue, 1.f, 1.f));
      this->hue += this->should_react_once ? this->hue_add_reactive : 0.f;
      break;
    }

    if (this->hue > 1.f)
    {
      this->hue = 0.f;    
    }

    this->should_react_once = false;
    this->should_react = false;
  }

  unsigned char pin_r = 0;
  unsigned char pin_g = 0;
  unsigned char pin_b = 0;
  bool should_react = false;
  bool should_react_once = false;
  bool active = true;
  int reactive = 2;
  int colormode = 1;
  c_color color = c_color(255, 0, 150);
  float hue = 0.f;
  float hue_add_fast = 0.001f;
  float hue_add_slow = 0.0001f;
  float hue_add_reactive = 0.1f;
};

namespace pins
{
  //pin7 = key1
  //pin2 = key2
  //pin3 = key3
  //pin6 = key4
  //pin4 = key5
  //pin5 = key6
  
  unsigned char keys[6] =
  {
    7, 2, 3, 6, 4, 5
  };
  unsigned char led_r = 11;
  unsigned char led_g = 8;
  unsigned char led_b = 12;
  unsigned char led_power = 9;

  void setup()
  {
    for (unsigned int i = 0; i < 6; i++)
    {
      pinMode(pins::keys[i], INPUT_PULLUP);
    }

    pinMode(pins::led_r, OUTPUT);
    pinMode(pins::led_g, OUTPUT);
    pinMode(pins::led_b, OUTPUT);
    pinMode(pins::led_power, OUTPUT);
  }
}

c_led led;
c_key keys[6];

void read_config()
{
  if (!Serial.available())
  {
    return;
  }
  
  String buffer = Serial.readStringUntil('\n');

  if (buffer.length() > 2 && buffer.charAt(0) == 'c')
  {
    unsigned int config_index = 0;
    String current_word = "";
    
    for (unsigned int i = 1; i < buffer.length(); i++)
    {
      char current_char = buffer.charAt(i);
      
      if (current_char == ',')
      {
        if (current_word.length())
        {
          switch(config_index)
          {
          case 0:
            led.active = current_word.toInt();
            break;
          case 1:
            led.reactive = current_word.toInt();
            break;
          case 2:
            led.colormode = current_word.toInt();
            break;
          case 3:
            led.color.r = current_word.toInt();
            break;
          case 4:
            led.color.g = current_word.toInt();
            break;
          case 5:
            led.color.b = current_word.toInt();
            break;
          }
        }
        
        current_word = "";
        
        config_index++;
      }
      else
      {
        current_word += current_char;
      }
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(1000);
  
  pins::setup();
  led.setup(pins::led_r, pins::led_g, pins::led_b);

  for (unsigned int i = 0; i < 6; i++)
  {
    keys[i].setup(pins::keys[i]);
  }
}

void loop()
{
  digitalWrite(pins::led_power, HIGH);

  read_config();

  for (unsigned int i = 0; i < 6; i++) 
  {
    keys[i].update();

    if (keys[i].clicked_last_update)
    {
      led.should_react_once = true;
      
      Serial.print(String(i) + "d\n");
    }
    else if (keys[i].released_last_update)
    {
      Serial.print(String(i) + "u\n");
    }
    
    if (keys[i].down)
    {
      led.should_react = true;
    }
  }

  led.update();
  
  delay(1);
}
