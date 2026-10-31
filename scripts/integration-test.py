#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
import json,urllib.request,http.cookiejar,socket,tempfile,os,subprocess,time,sys
base=sys.argv[1] if len(sys.argv)>1 else 'http://127.0.0.1:8080'
for _ in range(30):
    try:
        urllib.request.urlopen(base+'/login',timeout=1).close();break
    except OSError:time.sleep(1)
else:raise SystemExit('Web service did not become ready')
credentials={'username':os.environ['KELD_TEST_USERNAME'],'password':os.environ['KELD_TEST_PASSWORD']}
client=urllib.request.build_opener(urllib.request.HTTPCookieProcessor(http.cookiejar.CookieJar()))
def api(path,body=None):
    request=urllib.request.Request(base+path,data=json.dumps(body or {}).encode(),headers={'Content-Type':'application/json'})
    result=json.load(client.open(request,timeout=20))
    if result['status']!=0:raise RuntimeError(path+': '+result.get('message','failed'))
    return result.get('data')
def await_event(module,predicate):
    for _ in range(20):
        for event in api('/'+module+'/listEvents',{'limit':200,'offset':0}) or []:
            if predicate(event):return event
        time.sleep(.25)
    raise RuntimeError(module+' real kernel event not received')
api('/auth/login',{'username':credentials['username'],'password':credentials['password']})
assert api('/ctrl/status')['coreOnline']
assert api('/auth/showCurrentUserInfo')['username']==credentials['username']
listing=api('/file/browse',{'path':os.environ.get('KELD_TEST_ROOT',os.path.dirname(os.path.dirname(os.path.abspath(__file__))))})
assert any(entry['name']=='core' and entry['directory'] for entry in listing['entries'])
print('Authenticated session, core connection, host filesystem browsing: OK',flush=True)
if '--full' not in sys.argv:
    api('/auth/logout');sys.exit(0)
file_id=net_id=None
fixture=tempfile.NamedTemporaryFile(prefix='keld-smoke-',delete=False)
fixture.write(b'initial\n');fixture.close()
listener=None
try:
    api('/process/updateWorkMode',{'judge':1})
    api('/process/enableModule')
    time.sleep(.2)
    subprocess.run(['/usr/bin/true'],check=True)
    event=await_event('process',lambda item:item['binary'].endswith('/true'))
    api('/process/updateEventStatus',{'id':event['id'],'status':2})
    api('/process/updateEventStatus',{'id':event['id'],'status':1})
    api('/process/deleteEvents',{'id':event['id']})
    api('/process/disableModule');api('/process/updateWorkMode',{'judge':0})
    print('Process kernel audit, trust/untrust, event deletion: OK',flush=True)
    api('/file/addPolicy',{'path':fixture.name,'perm':48})
    policies=api('/file/listPolicies',{'limit':200,'offset':0}) or []
    file_id=next(row['id'] for row in policies if row['path']==fixture.name)
    api('/file/enableModule')
    with open(fixture.name,'a') as out:out.write('audit\n')
    with open(fixture.name) as source:assert 'audit' in source.read()
    event=await_event('file',lambda item:item['path']==fixture.name)
    api('/file/updateEventStatus',{'id':event['id'],'status':1})
    api('/file/updatePolicy',{'id':file_id,'perm':34})
    try:
        with open(fixture.name,'a') as out:out.write('must be blocked\n')
    except PermissionError:pass
    else:raise RuntimeError('File write protection did not block write')
    api('/file/deletePolicy',{'id':file_id});file_id=None
    api('/file/disableModule')
    for item in api('/file/listEvents',{'limit':200,'offset':0}) or []:
        if item['path']==fixture.name:api('/file/deleteEvent',{'id':item['id']})
    print('File kernel audit, write blocking, policy management, event handling: OK',flush=True)
    listener=socket.socket(socket.AF_INET,socket.SOCK_STREAM);listener.bind(('127.0.0.1',0));listener.listen(1)
    port=listener.getsockname()[1]
    rule={'priority':-128,'addr':{'src':{'begin':'0.0.0.0','end':'255.255.255.255'},'dst':{'begin':'127.0.0.1','end':'127.0.0.1'}},'protocol':{'begin':6,'end':6},'port':{'src':{'begin':0,'end':65535},'dst':{'begin':port,'end':port}},'flags':2,'response':3}
    api('/net/addPolicy',rule)
    policies=api('/net/listPolicies',{'limit':200,'offset':0}) or []
    net_id=next(row['id'] for row in policies if row['port']['dst']['begin']==port)
    api('/net/enableModule')
    with socket.create_connection(('127.0.0.1',port),timeout=3):pass
    event=await_event('net',lambda item:item['dport']==port)
    api('/net/updateEventStatus',{'id':event['id'],'status':1})
    api('/net/deletePolicy',{'id':net_id});net_id=None
    rule['response']=2
    api('/net/addPolicy',rule)
    policies=api('/net/listPolicies',{'limit':200,'offset':0}) or []
    net_id=next(row['id'] for row in policies if row['port']['dst']['begin']==port)
    try:
        with socket.create_connection(('127.0.0.1',port),timeout=1):pass
    except OSError:pass
    else:raise RuntimeError('Network drop policy did not block connection')
    api('/net/deletePolicy',{'id':net_id});net_id=None
    api('/net/disableModule')
    for item in api('/net/listEvents',{'limit':200,'offset':0}) or []:
        if item['dport']==port:api('/net/deleteEvent',{'id':item['id']})
    print('Network kernel audit, connection blocking, policy management, event handling: OK',flush=True)
finally:
    for module in ('process','file','net'):
        try:api('/'+module+'/disableModule')
        except Exception as error:print('Cleanup failed:',module,str(error),flush=True)
    try:api('/process/updateWorkMode',{'judge':0})
    except Exception as error:print('Mode cleanup failed:',str(error),flush=True)
    for module,identity in (('file',file_id),('net',net_id)):
        if identity is not None:
            try:api('/'+module+'/deletePolicy',{'id':identity})
            except Exception as error:print('Policy cleanup failed:',module,str(error),flush=True)
    if listener:listener.close()
    os.unlink(fixture.name)
assert api('/ctrl/status')['coreOnline']
assert all(api('/'+module+'/showModuleStatus')['status']==0 for module in ('process','file','net'))
api('/auth/logout')
print('Final state: core online, test policies removed, modules restored to disabled defaults',flush=True)
