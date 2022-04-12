import { RequestHandler } from 'express';
import { generateToken, decodeToken } from '../token';
import config from '../../config';
const { cookieOptions } = config.auth;
const debug = require('debug')('auth');

declare global {
  export type UserTokenData = {
    id: number;
    displayName: string;
  };

  namespace Express {
    export interface Request {
      user?: UserTokenData;
    }
  }
}

export function jwtParser(options?: any): RequestHandler {
  return async function jwtParser(req, res, next) {
    try {
      const authHeader = req.headers['authorization'];
      const token = authHeader && authHeader.split(' ')[1];

      if (token == null) {
        return next();
      }

      const decoded = await decodeToken(token);
      const { user } = decoded;

      // re-issue token when its age is over
      if (Date.now() / 1000 - decoded.iat > cookieOptions.maxAge) {
        const freshToken = await generateToken({ user }, 'user');
        res.cookie('access_token', freshToken, cookieOptions);
      }

      // `req.user` indicates if the given request is authorized.
      req.user = user;
    } catch (e) {
      debug(e);
      req.user = null;
    }

    next();
  };
}
