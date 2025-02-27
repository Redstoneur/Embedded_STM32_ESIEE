from pydantic import BaseModel, Field


class LedRGB(BaseModel):
    Red: int = Field(..., ge=0, le=255)  # entre 0 et 255
    Green: int = Field(..., ge=0, le=255)  # entre 0 et 255
    Blue: int = Field(..., ge=0, le=255)  # entre 0 et 255
    State: bool  # True = allumé, False = éteint

    def color(self, red: int, green: int, blue: int):
        self.Red = max(0, min(255, red))
        self.Green = max(0, min(255, green))
        self.Blue = max(0, min(255, blue))

    @staticmethod
    def type() -> dict:
        return {
            "red": int,
            "green": int,
            "blue": int,
            "state": bool
        }

    @staticmethod
    def from_dict(data: dict) -> "LedRGB":
        return LedRGB(
            Red=data["red"],
            Green=data["green"],
            Blue=data["blue"],
            State=data["state"]
        )
