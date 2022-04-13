/*
 * Copyright 2022-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


import { Router } from 'express';
import { ServiceData, ServicesDB } from '../db/Services';
import DAO from '../db/dao';
const debug = require('debug')('service');

const State = {
  RUN: 'run',
  STOP: 'stop'
} as const;
type State = typeof State[keyof typeof State];


export class Service {
  private readonly name: string;
  private path: string;
  private state: State;

  constructor(data: ServiceData, router: Router) {
    this.name = data.name;
    this.path = data.path;
    this.state = State.STOP;

    router.get(`/${this.name}`, (req, res) => {
      debug(`${this.name} state is ${this.state}.`);

      if (this.state === State.RUN) {
        debug(`render service: ${this.name}: ${this.path}`);
        res.render(this.path, { name: this.name });
      } else {
        res.status(404).send('not found');
      }
    });
  }

  public setPath(path: string) {
    this.path = path;
  }

  public run() {
    if (this.state === State.RUN) {
      return;
    }

    debug(`run service: ${this.name}(${this.path})`);

    this.state = State.RUN;
  }

  public stop() {
    if (this.state === State.STOP) {
      return;
    }

    debug(`stop service: ${this.name}`);

    this.state = State.STOP;
  }
}

export class Services {
  private static instance: Services;
  private router: Router;
  private appList: Map<string, Service>;
  private viewEngine: string;
  private isRunning: boolean;
  private servicesDB: ServicesDB;

  private constructor() {
    this.appList = new Map<string, Service>();
    this.viewEngine = 'ejs';
    this.isRunning = false;
    this.servicesDB = new ServicesDB(DAO.knex());
  }

  public static getInstance(): Services {
    return this.instance || (this.instance = new this())
  }

  public getViewEngine(): string {
    return this.viewEngine;
  }

  public async start(router: Router) {
    this.router = router;
    this.isRunning = true;

    try {
      const servicesData = await this.servicesDB.getDataAll();
      servicesData.forEach(function (data) {
        this.addService(data);
      }, this);
    } catch(e) {
      debug('cannot start service app.');
    }
  }

  public addService(data: ServiceData) {
    if (!this.isRunning) {
      throw new Error("Service is not start!");
    }

    let service;
    if (this.appList.has(data.name)) {
      service = this.appList.get(data.name);
      service.setPath(data.path);
    } else {
      service = new Service(data, this.router);
      this.appList.set(data.name, service);
    }

    service.run();
  }

  public deleteService(name: string) {
    if (!this.isRunning) {
      throw new Error("Service is not start!");
    }

    if (!this.appList.has(name)) {
      throw new Error(`${name} is not exists.`);
    }

    const service = this.appList.get(name);
    service.stop();
  }
}

export default Services;
