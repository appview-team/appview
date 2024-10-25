const documentPreface =
`In AppView, events are structured according to one pattern, and metrics are structured according to another. These patterns are defined rigorously, in validatable [JSON Schema](https://json-schema.org/). 

Three [definitions schemas](https://github.com/appview-team/appview/tree/master/docs/schemas/definitions) govern the basic patterns. Then there is an individual schema for each event and metric, documented below. The definitions schemas define the elements that can be present in individual event and metric schemas, as well as the overall structures into which those elements fit. 

When we say "the AppView schema," we mean the [whole set](https://github.com/appview-team/appview/tree/master/docs/schemas/) of schemas. The AppView schema now in use was introduced in AppView 1.0.1.

A few event and metric schema elements, namely \`title\` and \`description\`, have placeholder values. In the future, we might make these more informative. They are essentially "internal documentation" within the schemas and do not affect how the schemas function in AppView. In the event that you develop any code that depends on AppView schemas, be aware that the content of \`title\` and \`description\` fields may evolve.

For more about how events and metrics work in AppView, see [this](events-and-metrics) overview.`;

const tocOrder = [
  'event.fs.open',
  'event.fs.close',
  'event.fs.duration',
  'event.fs.error',
  'event.fs.read',
  'event.fs.write',
  'event.fs.delete',
  'event.fs.seek',
  'event.fs.stat',
  'event.net.open',
  'event.net.close',
  'event.net.duration',
  'event.net.error',
  'event.net.rx',
  'event.net.tx',
  'event.net.app',
  'event.net.port',
  'event.net.tcp',
  'event.net.udp',
  'event.net.other',
  'event.http.req',
  'event.http.resp',
  'event.dns.req',
  'event.dns.resp',
  'event.file',
  'event.stdout',
  'event.console',
  'event.stderr',
  'event.notice',
  'event.start.msg',
  'event.sec.file',
  'event.sec.dns',
  'event.sec.got',
  'event.sec.conn',

  'metric.fs.open',
  'metric.fs.close',
  'metric.fs.duration',
  'metric.fs.error',
  'metric.fs.read',
  'metric.fs.write',
  'metric.fs.seek',
  'metric.fs.stat',
  'metric.net.open',
  'metric.net.close',
  'metric.net.duration',
  'metric.net.error',
  'metric.net.rx',
  'metric.net.tx',
  'metric.net.port',
  'metric.net.tcp',
  'metric.net.udp',
  'metric.net.other',
  'metric.http.req',
  'metric.http.req.content.length',
  'metric.http.resp.content.length',
  'metric.http.duration.client',
  'metric.http.duration.server',
  'metric.dns.req',
  'metric.proc.fd',
  'metric.proc.thread',
  'metric.proc.start',
  'metric.proc.child',
  'metric.proc.cpu',
  'metric.proc.cpu.perc',
  'metric.proc.mem',
];

const tocHeaders = {
  fs: 'Filesystem',
  net: 'Network',
  http: 'HTTP',
  dns: 'DNS',
  file: 'File',
  console: 'Console',
  notice: 'System Notification',
  start: `Start Message`,
  proc: 'Process',
  sec: 'Security'
}

module.exports = {
  documentPreface,
  tocOrder,
  tocHeaders
}
