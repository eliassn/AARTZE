"""Minimal NodeGraphQt integration scaffold.

Defines sample nodes (BeginPlay, Print, SetTransform) and utility functions to
create a graph widget, save/load JSON, and compile to a tiny bytecode.
This file is not yet wired into the main UI, but can be embedded into a
QDockWidget or page as needed.
"""
from __future__ import annotations
from NodeGraphQt import NodeGraph, BaseNode  # type: ignore


class BeginPlayNode(BaseNode):
    __identifier__ = 'aartze.events'
    NODE_NAME = 'BeginPlay'
    def __init__(self):
        super().__init__()
        self.add_output('exec')


class PrintNode(BaseNode):
    __identifier__ = 'aartze.util'
    NODE_NAME = 'Print'
    def __init__(self):
        super().__init__()
        self.add_input('exec')
        self.add_output('exec')
        self.add_text_input('text', 'Hello')


class SetTransformNode(BaseNode):
    __identifier__ = 'aartze.scene'
    NODE_NAME = 'SetTransform'
    def __init__(self):
        super().__init__()
        self.add_input('exec')
        self.add_output('exec')
        self.add_text_input('entity', 'Cube')
        self.add_text_input('pos', '0,0,0')
        self.add_text_input('rot', '0,0,0')


def make_graph() -> NodeGraph:
    graph = NodeGraph()
    graph.register_node(BeginPlayNode)
    graph.register_node(PrintNode)
    graph.register_node(SetTransformNode)
    return graph

