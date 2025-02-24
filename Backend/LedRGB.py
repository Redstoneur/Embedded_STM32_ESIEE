class LedRGB:
    Red: int  # entre 0 et 255
    Green: int  # entre 0 et 255
    Blue: int  # entre 0 et 255
    State: bool  # True = allumé, False = éteint

    def __init__(self, red: int, green: int, blue: int, state: bool):
        self.color(red, green, blue)
        self.State = state

    def __dict__(self):
        return {
            "red": self.Red,
            "green": self.Green,
            "blue": self.Blue,
            "state": self.State
        }

    def __str__(self):
        return f"Led RGB : {self.__dict__()}"

    def color(self, red: int, green: int, blue: int):
        self.Red = red if red >= 0 else 0 if red <= 255 else 255
        self.Green = green if green >= 0 else 0 if green <= 255 else 255
        self.Blue = blue if blue >= 0 else 0 if blue <= 255 else 255

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
            red=data["red"],
            green=data["green"],
            blue=data["blue"],
            state=data["state"]
        )
