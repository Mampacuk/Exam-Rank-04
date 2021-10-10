#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

typedef struct	s_cmd
{
	char	**args;
	int		in;
	int		out;
}				t_cmd;

typedef struct	s_data
{
	int		cmdnum;
	int		segments;
	t_cmd	*cmds;
	char	**env;
	pid_t	*family;
}				t_data;

t_data	g_data;

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	if (!str)
		return (i);
	while (str[i])
		i++;
	return (i);
}

void	ft_puterr(char *msg)
{
	write(2, msg, ft_strlen(msg));
}

int	ft_exit(char *msg, int callkill)
{
	ft_puterr(msg);
	if (callkill)
		kill(0, SIGINT);
	exit(1);
}

int	ft_exec(t_cmd *cmd)
{
	// printf("pid%d about to dup2 (%d, 0) and dup2(%d, 1)\n", getpid(), cmd->in, cmd->out);
	if (dup2(cmd->in, 0) == -1 || dup2(cmd->out, 1) == -1)
		ft_exit("error: fatal\n", 1);
	execve(cmd->args[0], cmd->args, g_data.env);
	ft_puterr("error: cannot execute ");
	ft_puterr(cmd->args[0]);
	ft_puterr("\n");
	exit(127);
}

int	main(int argc, char **argv, char **environ)
{
	// printf("MAIN PID:%d\n", getpid());
	(void)argc;
	g_data.env = environ;
	g_data.segments = 1;
	int k = 0;
	while (argv[k])
		if (!strcmp(argv[k++], ";"))
			g_data.segments++;
	argv++;
	// printf("initally shifted argv is |%s|\n", *argv);
	while (*argv && g_data.segments)
	{
		if (!strcmp(*argv, ";"))
		{
			g_data.segments--;
			argv++;
			continue ;
		}
		// PARSING START
		g_data.cmdnum = 1;
		int	i = 0;
		while (argv[i] && strcmp(argv[i], ";"))
		{
			if (!strcmp("|", argv[i]))
				g_data.cmdnum++;
			i++;
		}
		g_data.cmds = malloc(sizeof(t_cmd) * g_data.cmdnum);
		g_data.family = malloc(sizeof(pid_t) * g_data.cmdnum);
		if (!g_data.cmds || !g_data.family)
			ft_exit("error: fatal\n", 0);
		int 	currcomm = 0;
		i = 0;
		char	**beg = argv;
		int		cont = 1;
		while (cont)
		{
			// printf("beg[%d] = |%s|\n", i, beg[i]);
			g_data.cmds[currcomm].args = beg;
			// printf("g_data.cmds[%d].args = beg -> |%s|\n", currcomm, *beg);
			if (!beg[i] || !strcmp(beg[i], "|") || !strcmp(beg[i], ";"))
			{
				// printf("found | or ; or NULL\n");
				if (beg[i])
				{
					if (!strcmp(beg[i], ";"))
						cont = 0;
				}
				else
					cont = 0;	
				beg[i] = NULL;
				currcomm++;
				// printf("currcom now %d\n", currcomm);
				if (cont)
					beg += i + 1;
				// printf("ended parsing a command; beg pointing at |%s|\n", *beg);
				i = 0;
			}
			else
				i++;
		}
		// PARSING END

		// PIPING START
		i = 0;
		while (i < g_data.cmdnum)
		{
			g_data.cmds[i].in = 0;
			g_data.cmds[i].out = 1;
			if (i)
			{
				int	pipefd[2];
				if (pipe(pipefd))
					ft_exit("error: fatal\n", 0);
				// printf("piped write-end:%d read-end:%d\n", pipefd[1], pipefd[0]);
				g_data.cmds[i - 1].out = pipefd[1];
				g_data.cmds[i].in = pipefd[0];
			}
			i++;
		}
		// PIPING END
		// for (int i = 0; i < g_data.cmdnum; i++)
		// {
		// 	printf("cmd:%d in:%d out:%d\n\n", i, g_data.cmds[i].in, g_data.cmds[i].out);
		// 	for (int j = 0; g_data.cmds[i].args[j]; j++)
		// 		printf("arg[%d]:|%s|\n", j, g_data.cmds[i].args[j]);
		// 	printf("\n");
		// }
		// FORKING START
		// printf("first arg:%s\n", g_data.cmds[0].args[0]);
		// exit(0);
			// printf("pid%d\t !!!! IS FIRST CD? !!! \n", getpid());
		if (strcmp(g_data.cmds[0].args[0], "cd"))
		{
			int z = 0;
			g_data.family[z] = 1;
			while (z < g_data.cmdnum)
			{
				g_data.family[z] = fork();
				if (g_data.family[z] == -1)
					ft_exit("error: fatal\n", 1);
				else if (g_data.family[z] == 0)
					ft_exec(g_data.cmds + z);
				// else
				// 	printf("FORKED %d\n", g_data.family[z]);
				waitpid(g_data.family[z], NULL, 0);
				if (g_data.cmds[z].in != 0)
					close(g_data.cmds[z].in);
				if (g_data.cmds[z].out != 1)
					close(g_data.cmds[z].out);
				z++;
			}
		}
		else
		{
			if (!g_data.cmds[0].args[1] || g_data.cmds[0].args[2])
				ft_puterr("error: cd: bad arguments\n");
			if (chdir(g_data.cmds[0].args[1]) == -1)
			{
				ft_puterr("error: cd: cannot change directory to ");
				ft_puterr(g_data.cmds[0].args[1]);
				ft_puterr("\n");
			}
		}
		// FORKING END
		i = 0;
		while (g_data.cmds[g_data.cmdnum - 1].args[i])
			i++;
		if (g_data.segments > 1)
			argv = &g_data.cmds[g_data.cmdnum - 1].args[i] + 1;
		free(g_data.cmds);
		free(g_data.family);
		g_data.segments--;
	}
}