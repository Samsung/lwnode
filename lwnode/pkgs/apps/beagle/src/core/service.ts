import util from 'util';
import utils from 'lib/utils';

export default class Service {
  private broker = null;
  private name: string = '';
  private fullName: string = '';
  private version = null;
  private settings = null;
  private schema = null;

  constructor(broker, schema) {
    this.broker = broker;

    if (schema) {
      this.parseServiceSchema(schema);
    }
  }

  parseServiceSchema(schema) {
    this.name = schema.name;
    this.version = schema.version;
    this.settings = schema.settings || {};
    this.schema = schema;
    this.fullName = Service.getVersionedFullName(this.name, this.version);

    // Service item for Registry
    const serviceSpecification = {
      name: this.name,
      version: this.version,
      fullName: this.fullName,
      actions: {},
      events: {},
    };

    // Register actions
    if (utils.isObject(schema.actions)) {
      for (const name in schema.actions) {
        const action = schema.actions[name];

        let innerAction = this.createAction(action, name);

        serviceSpecification.actions[innerAction.name] = innerAction;

        // todo
      }
    }
  }

  private createAction(name, handler) {
    let action = {
      name: this.fullName + '.' + name,
      handler: util.promisify(handler.bind(this)),
      service: this,
    };

    return action;
  }

  static getVersionedFullName(name, version) {
    if (version != null)
      return (
        (typeof version == 'number' ? 'v' + version : version) + '.' + name
      );

    return name;
  }
}
