# AI World Generation Utilities

This module hosts experimental AI helpers for generating worlds based on
`NAFEngine` templates.

## Interface

`GenerateWorld(prompt)` builds terrain and entity lists using templates in
`NAFEngine/World`. Each call also stores the prompt as a custom part to
`learning_data.txt` so that later generations can incorporate previous
additions.

## Training Requirements

To train models that guide world generation, install one of the supported
frameworks:

- [PyTorch](https://pytorch.org/) >= 1.9
- [TensorFlow](https://www.tensorflow.org/) >= 2.11

Training data is persisted in `learning_data.txt`, which can be consumed by
future training scripts.
