from IPS.EventMan.ievent_mediator import IEventMediator

class DataEventMediator(IEventMediator):
    def __init__(self):
        self._components = []

    def register(self, component):
        self._components.append(component)

    def notify_event(self, sender, event):
        for c in self._components:
            if c != sender:
                c.consume_event(sender, event)
