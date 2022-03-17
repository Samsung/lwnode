import Koa from 'koa';
import Router from 'koa-router';
import { ServiceBroker } from './core';
import MathServiceScheme from './service/math.service';

const app = new Koa();
const router = new Router();
const broker = new ServiceBroker();

broker.createService(MathServiceScheme);
broker.start();

router.get('/', async (ctx) => {
  try {
    const res = await broker.exec('greeter.hi', { param: 'param1' });
    ctx.body = res;
  } catch (error) {
    // todo: handling errors
  }
});

app.use(router.routes());
app.listen(3000, () => console.log('listening'));
