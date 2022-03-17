// import { getNodeID } from 'lib/utils';
import Service from './service';

interface ILogger {
  info(arg: string, ...args: any[]): void;
  error(arg: string, ...args: any[]): void;
}

class Logger implements ILogger {
  error(arg: string, ...args: any[]) {
    console.error.apply(null, [arg, ...args]);
  }
  info(arg: string, ...args: any[]) {
    console.log.apply(null, [arg, ...args]);
  }
}

class Registry {
  constructor(broker: ServiceBroker) {

  }
}

class ServiceBroker {
  private logger: Logger;
  // private nodeID: string;
  private registry: Registry;
  private isStarted: boolean = false;

  constructor() {
    // this.nodeID = getNodeID();
    // Service registry
    this.registry = new Registry(this);
    // this.registry.init(this);
    this.logger = new Logger();

    process.on('beforeExit', this.closeHandler);
    process.on('exit', this.closeHandler);
    process.on('SIGINT', this.closeHandler);
    process.on('SIGTERM', this.closeHandler);
  }

  private closeHandler() {
    this.stop()
      .catch((err) => this.logger.error(err))
      .then(() => process.exit(0));
  }

  private restartService(service) {}

  createService(schema) {
    let service = new Service(this, schema);

    if (this.isStarted) {
      this.restartService(service);
    }
    return service;
  }

  async start() {}

  async exec(actionName: string, params?: Object, options?: Object) {}

  async stop() {}
}

export default ServiceBroker;
