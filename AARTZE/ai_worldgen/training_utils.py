"""Utility helpers for storing training samples for the world generator."""
from pathlib import Path

data_file = Path(__file__).with_name("learning_data.txt")


def save_training_sample(name: str, detail: str) -> None:
    """Append a custom part to the learning data file."""
    with data_file.open("a", encoding="utf-8") as f:
        f.write(f"{name}:{detail}\n")


if __name__ == "__main__":
    import sys

    if len(sys.argv) > 2:
        save_training_sample(sys.argv[1], sys.argv[2])
