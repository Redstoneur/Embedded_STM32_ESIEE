from pydantic import BaseModel

from LedRGB import LedRGB


class CapteurInformation(BaseModel):
    Temperature: float  # en degrés Celsius
    Humidity: float  # en pourcentage
    RGB: LedRGB  # état de la LED RGB
    Led: bool  # état de la LED
    Buzzer: bool  # état du buzzer
    Button: bool  # état du button

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
            Temperature=data["Temperature"],
            Humidity=data["Humidity"],
            RGB=LedRGB.from_dict(data["RGB"]),
            Led=data["Led"],
            Buzzer=data["Buzzer"],
            Button=data["Button"]
        )
