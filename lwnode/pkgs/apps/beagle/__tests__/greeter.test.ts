import { Greeter } from 'greeter';

test('Should greet with message', () => {
  const greeter = new Greeter('Beagle');
  expect(greeter.greet()).toBe('Bonjour, Beagle!');
});
