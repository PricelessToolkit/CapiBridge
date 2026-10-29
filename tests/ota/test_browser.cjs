// Run with Node.js and Playwright installed. No device is contacted.
const { chromium } = require('playwright');
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const http = require('node:http');
const source = fs.readFileSync(path.join(__dirname, '../../code/Gateway/ESP1/web_ui.h'), 'utf8');
const html = source.split('R"rawliteral(')[1].split(')rawliteral";')[0];
const server = http.createServer((req, res) => { res.end(html); });
const settings = { wifiSsid:'Client WiFi', mqttServer:'broker', mqttPort:1883, gatewayKey:'xy', encryptionKey:['0x11','0x22'], band:868, loraModuleLabel:'SX1262', loraTxPower:20, loraSignalBandwidth:250, loraSpreadingFactor:10, loraCodingRate:5, loraSyncWord:18, loraPreambleLength:12 };
(async()=>{
  await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
  const browser = await chromium.launch({headless:true});
  try {
    const page = await browser.newPage({viewport:{width:1280,height:1000}});
    const errors=[];page.on('pageerror',e=>errors.push(e.message));
    const dialogs=[];page.on('dialog',async dialog=>{dialogs.push(dialog.type());await dialog.dismiss()});
    let uploadCount=0, mode='error', maxSize=1310720;
    await page.route('**/api/**', async route=>{
      const req=route.request(), endpoint=new URL(req.url()).pathname;
      let result={};
      if(endpoint==='/api/settings')result=settings;
      if(endpoint==='/api/status')result={firmwareVersion:'V1.4.0',radioReady:true,wifiConnected:true};
      if(endpoint==='/api/traffic')result={items:[],serialLines:[],nodes:[]};
      if(endpoint==='/api/firmware'){
        if(req.method()==='POST'){
          uploadCount++;
          assert.equal(req.headers()['x-capibridge-ota'],'ESP1');
          assert.equal(req.headers()['x-firmware-size'],'128');
          assert.match(req.postDataBuffer().toString(),/name="firmware"; filename="ESP1.ino.bin"/);
          await route.fulfill({status:mode==='error'?400:200,contentType:'application/json',body:JSON.stringify(mode==='error'?{ok:false,error:'This file is not OTA-capable CapiBridge ESP1 firmware.'}:{ok:true,rebootScheduled:true})});return;
        }
        result={target:'ESP1',version:'V1.4.0',available:maxSize>0,maxSize};
      }
      await route.fulfill({contentType:'application/json',body:JSON.stringify(result)});
    });
    const url=`http://127.0.0.1:${server.address().port}/#/settings`;
    await page.goto(url);
    await page.waitForFunction(()=>document.getElementById('firmwareVersion').textContent.includes('V1.4.0'));
    assert.equal(await page.locator('#firmwareUpdate').isDisabled(),true);
    const file={name:'ESP1.ino.bin',mimeType:'application/octet-stream',buffer:Buffer.alloc(128)};
    await page.locator('#firmwareFile').setInputFiles(file);
    await page.locator('#wifiSsid').fill('Unsaved network');
    await page.locator('#firmwareUpdate').click();
    await page.getByText('Save & Reboot your settings, or Reload them, before updating firmware.').waitFor();
    assert.equal(uploadCount,0);
    await page.locator('#reloadSettings').click();
    await page.waitForFunction(()=>document.getElementById('wifiSsid').value==='Client WiFi');
    maxSize=64;
    await page.locator('#firmwareUpdate').click();
    await page.getByText('The selected firmware is empty or too large for the OTA slot.').waitFor();
    assert.equal(uploadCount,0);
    maxSize=1310720;
    await page.locator('#firmwareUpdate').click();
    await page.getByText('This file is not OTA-capable CapiBridge ESP1 firmware.').waitFor();
    assert.equal(uploadCount,1);
    assert.equal(await page.locator('#firmwareUpdate').isDisabled(),false);
    await page.locator('#firmwareCard').screenshot({path:'/tmp/capibridge-firmware-card.png'});
    mode='success';
    const reloaded=page.waitForEvent('domcontentloaded');
    await page.locator('#firmwareUpdate').click();
    await reloaded;
    assert.equal(uploadCount,2);
    await page.setViewportSize({width:390,height:844});
    await page.waitForFunction(()=>document.getElementById('firmwareVersion').textContent.includes('V1.4.0'));
    assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth),true);
    await page.waitForTimeout(350);
    await page.screenshot({path:'/tmp/capibridge-firmware-mobile.png',fullPage:true});
    maxSize=0;
    await page.locator('#reloadSettings').click();
    await page.getByText('No OTA slot available. Install an OTA partition scheme over USB first.').waitFor();
    assert.equal(await page.locator('#firmwareUpdate').isDisabled(),true);
    assert.deepEqual(errors,[]);
    assert.deepEqual(dialogs,[],'Automatic reconnect must not trigger a leave-page dialog');
    console.log('Browser checks passed: selection, unsaved settings, size limits, errors, multipart upload, reboot reconnect, no OTA slot, mobile layout.');
  } finally { await browser.close();server.close(); }
})().catch(error=>{console.error(error);server.close();process.exitCode=1});
