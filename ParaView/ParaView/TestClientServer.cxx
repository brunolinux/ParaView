#include <vtkMultiThreader.h>
#include <TestClientServer.h>
#include <vtkstd/string>

void RunServer(void * p)
{
  vtkMultiThreader::ThreadInfo* info 
    = static_cast< vtkMultiThreader::ThreadInfo*>(p);
  char* path = static_cast<char*>(info->UserData);
  vtkstd::string run = path;
  run += " --server";
  cout << "Running:" <<  run.c_str() << "\n";
  cout.flush();
  system(run.c_str());
}


int main(int ac, char* av[])
{
  vtkstd::string path = PARAVIEW_BINARY_DIR;
  path += "/bin";
#ifdef  CMAKE_INTDIR
  path += "/" CMAKE_INTDIR;
#endif
  path += "/paraview";
  vtkMultiThreader* thread = vtkMultiThreader::New();
  thread->SetNumberOfThreads(2);
  int id = 
    thread->SpawnThread((vtkThreadFunctionType)RunServer, (void*)path.c_str());
  Sleep(2000);
  path += " --client";
  for(int i =1; i < ac; ++i)
    {
    path += " ";
    path += av[i];
    path += " ";
    }
  cout << "Running:" <<  path.c_str() << "\n";
  cout.flush();
  int ret = system(path.c_str());
  thread->TerminateThread(ret);
  thread->Delete();
  return ret;
}
