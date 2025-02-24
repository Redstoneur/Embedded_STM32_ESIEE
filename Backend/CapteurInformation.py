from pydantic import BaseModel

from LedRGB import LedRGB


class CapteurInformation(BaseModel):
    Temperature: float  # en degrés Celsius
    Humidity: float  # en pourcentage
    RGB: LedRGB  # état de la LED RGB
    Led: bool  # état de la LED
    Buzzer: bool  # état du buzzer
    Button: bool  # état du button

    def __init__(self, temperature: float, humidity: float, rgb: LedRGB,
                 led: bool, buzzer: bool, button: bool):
        self.Temperature = temperature
        self.Humidity = humidity
        self.RGB = rgb
        self.Led = led
        self.Buzzer = buzzer
        self.Button = button

    def __dict__(self) -> dict:
        return {
            "Temperature": self.Temperature,
            "humidity": self.Humidity,
            "LedRGB": self.RGB.__dict__(),
            "Led": self.Led,
            "Buzzer": self.Buzzer,
            "Button": self.Button
        }

    def __str__(self) -> str:
        return f"Capteur d'information : {self.__dict__()}"

    @staticmethod
    def type() -> dict:
        return {
            "Temperature": float,
            "Humidity": float,
            "RGB": LedRGB.type(),
            "Led": bool,
            "Buzzer": bool,
            "Button": bool
        }

    @staticmethod
    def from_dict(data: dict) -> "CapteurInformation":
        return CapteurInformation(
            temperature=data["Temperature"],
            humidity=data["Humidity"],
            rgb=LedRGB.from_dict(data["RGB"]),
            led=data["Led"],
            buzzer=data["Buzzer"],
            button=data["Button"]
        )
