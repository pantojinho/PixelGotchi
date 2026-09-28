// Código real do instalador; somente biblioteca serial, DOM e rede são substituídos.
const vm = require('node:vm');
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {webcrypto,createHash} = require('node:crypto');
const source=fs.readFileSync(path.join(__dirname,'../preview/install.js'),'utf8');
const bytes=Buffer.alloc(65536);bytes[0]=0xe9;
const info={version:'test',path:'firmware/PixelGotchi-esp32s3-test.bin',size:bytes.length,sha256:createHash('sha256').update(bytes).digest('hex'),chipFamily:'ESP32-S3',flashSize:4194304};
const manifest={version:'test',builds:[{chipFamily:'ESP32-S3',parts:[{path:info.path,offset:0}]}]};
async function scenario(kind) {
  const elements={};
  for(const name of ['#support','#install','#consent','esp-web-install-button','#retry','#version'])
    elements[name]={disabled:true,checked:false,hidden:false,textContent:'',events:{},addEventListener(n,fn){this.events[n]=fn;}};
  const responses = { 'manifest.json':manifest, 'firmware-info.json':{...info} };
  if(kind==='mismatch') responses['firmware-info.json'].version='other';
  const context=vm.createContext({
    document:{querySelector:s=>elements[s]},window:{isSecureContext:kind!=='insecure',addEventListener(){}},
    navigator:kind==='unsupported'?{}:{serial:{}},location:{href:'http://localhost/install.html'},URL,Blob,
    crypto:webcrypto,Uint8Array,customElements:{whenDefined:async()=>{}},
    fetch:async url=>({ok:kind!=='missing',json:async()=>responses[url],arrayBuffer:async()=>{
      const data=Buffer.from(bytes);if(kind==='corrupt') data[100]^=1;
      return data.buffer.slice(data.byteOffset,data.byteOffset+data.byteLength);
    }})
  });
  vm.runInContext(source,context,{importModuleDynamically:async()=>{
    const module=new vm.SyntheticModule([],()=>{},{context});await module.link(()=>{});await module.evaluate();return module;
  }});
  for(let i=0;i<30;i++) await new Promise(resolve=>setImmediate(resolve));
  const button=elements['#install'],consent=elements['#consent'],widget=elements['esp-web-install-button'];
  assert(button.disabled,'sem confirmação não pode instalar');
  consent.checked=true;consent.events.change();
  if(kind==='valid') {
    assert.equal(button.disabled,false);
    const locked = await (await fetch(widget.manifest)).json();
    assert.equal(locked.new_install_prompt_erase,false);
    assert.equal(locked.new_install_improv_wait_time,0);
    const actual=Buffer.from(await (await fetch(locked.builds[0].parts[0].path)).arrayBuffer());
    assert.deepEqual(actual,bytes); // o upload usa os bytes validados, sem novo download
    URL.revokeObjectURL(locked.builds[0].parts[0].path);URL.revokeObjectURL(widget.manifest);
  } else {
    assert(button.disabled,`${kind}: pacote inválido habilitou USB`);
    assert.equal(widget.manifest,undefined);
  }
}
(async()=>{
  for(const kind of ['valid','mismatch','missing','corrupt','insecure','unsupported']) await scenario(kind);
  console.log('PASS: instalador, consentimento, SHA-256, arquivo fixado, pacote inconsistente/ausente e suporte');
})().catch(error=>{console.error(error);process.exitCode=1;});
