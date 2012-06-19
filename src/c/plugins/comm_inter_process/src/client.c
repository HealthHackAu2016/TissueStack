int		init_sock_comm_client(char *path)
{
  struct sockaddr_un address;
  int		socket_fd;
  int		nbytes;

  socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(socket_fd < 0)
    {
      printf("socket() failed\n");
      return 1;
    }
  memset(&address, 0, sizeof(struct sockaddr_un));
  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, 108, path);
  if(connect(socket_fd, (struct sockaddr *) &address,
	     sizeof(struct sockaddr_un)) != 0)
    {
      printf("connect() failed\n");
      return 1;
    }
  return (socket_fd);
}
