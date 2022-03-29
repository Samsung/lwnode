import jwt, { Jwt, JwtPayload } from 'jsonwebtoken';
import config from '../config';

const { privateKey: secret, signOptions } = config.auth;

export function generateToken(
  payload: string | Buffer | object,
  subject: string,
): Promise<string> {
  return new Promise((resolve, reject) => {
    jwt.sign(
      payload,
      secret,
      {
        subject,
        ...signOptions,
      },
      (error, token) => {
        if (error) reject(error);
        resolve(token);
      },
    );
  });
}

export function decodeToken(token: string): Promise<string | Jwt | JwtPayload> {
  return new Promise((resolve, reject) => {
    jwt.verify(token, secret, (error, decoded) => {
      if (error) reject(error);
      resolve(decoded);
    });
  });
}
