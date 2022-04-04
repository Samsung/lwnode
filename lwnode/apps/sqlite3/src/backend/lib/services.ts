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
  private readonly path: string;
  private state: State;

  constructor(data: ServiceData) {
    this.name = data.name;
    this.path = data.path;
    this.state = State.STOP;
  }

  public run(router: Router) {
    if (this.state === State.RUN) {
      return;
    }

    try {
      router.get(`/${this.name}`, (req, res) => {
        debug(`render service: ${this.name}`);
        res.render(this.path, { name: this.name });
      });
    } catch (error) {
      debug(`cannot start service: ${this.name}`);
      return;
    }

    debug(`run service: ${this.name}(${this.path})`);

    this.state = State.RUN;
  }
}

export class Services {
  private static instance: Services;
  private router: Router;
  private appList: Map<string, Service>;
  private viewEngine: string;
  private isRunning: boolean;

  private constructor() {
    this.appList = new Map<string, Service>();
    this.viewEngine = 'ejs';
    this.isRunning = false;
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

    const servicesDB = new ServicesDB(DAO.knex());
    const servicesData = await servicesDB.getDataAll();
    servicesData.forEach(function (data) {
      this.addService(data);
    }, this);
  }

  public addService(data: ServiceData) {
    if (this.appList.has(data.name)) {
      debug(`${data.name} already exists.`)
      return;
    }
    const service = new Service(data);
    this.appList.set(data.name, service);

    if (this.isRunning) {
      service.run(this.router);
    }
  }
}

export default Services;
