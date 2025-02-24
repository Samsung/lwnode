class MessagePort {
  #port;
  constructor(port) {
    this.#port = port;
  }

  postMessage(message) {
    if (typeof message !== "string") {
      throw new TypeError("The message argument must be a string");
    }
    return this.#port.postMessage(message);
  }

  get onmessage() {
    return this.#port.onmessage;
  }

  set onmessage(callback) {
    if (typeof callback !== "function") {
      throw new TypeError("The onmessage property must be a function");
    }
    this.#port.onmessage = callback;
  }
}

class Event {
  #type;

  constructor(type) {
    this.#type = type;
  }
  get type() {
    return this.#type;
  }
}

class MessageEvent extends Event {
  #options;

  constructor(type, options) {
    super(type);
    this.#options = Object.freeze(options);
  }
  get data() {
    return this.#options.data;
  }
  get origin() {
    return this.#options.origin;
  }
  get ports() {
    return this.#options.ports;
  }
}

function setupMessagePort(target, binding) {
  const port = new MessagePort(binding.MainMessagePort);

  Object.assign(target, {
    port,
    ref: () => {
      return binding.ref();
    },
    unref: () => {
      return binding.unref();
    },
    MessageEvent,
  });

  // onmessage
  Object.defineProperty(target, "onmessage", {
    __proto__: null,
    enumerable: false,
    configurable: false,
    get: () => {
      return port.onmessage;
    },
    set(newValue) {
      port.onmessage = newValue;
    },
  });

  Object.defineProperty(target, "postMessage", {
    __proto__: null,
    enumerable: false,
    configurable: false,
    writable: false,
    value: port.postMessage.bind(port),
  });
}

module.exports = {
  setupMessagePort,
};
