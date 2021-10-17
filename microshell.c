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
	int	i;

	(void)argc;
	g_data.env = environ;
	g_data.segments = 1;
	i = 0;
	while (argv[i])
		if (!strcmp(argv[i++], ";"))
			g_data.segments++;
	argv++;
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
		i = 0;
		while (argv[i] && strcmp(argv[i], ";"))
			if (!strcmp("|", argv[i++]))
				g_data.cmdnum++;
		g_data.cmds = malloc(sizeof(t_cmd) * g_data.cmdnum);
		g_data.family = malloc(sizeof(pid_t) * g_data.cmdnum);
		if (!g_data.cmds || !g_data.family)
			ft_exit("error: fatal\n", 0);
		int 	currcomm = 0;
		char	**beg = argv;
		int		cont = 1;
				i = 0;
		while (cont)
		{
			g_data.cmds[currcomm].args = beg;
			g_data.cmds[currcomm].in = 0;
			g_data.cmds[currcomm].out = 1;
			if (!beg[i] || !strcmp(beg[i], "|") || !strcmp(beg[i], ";"))
			{
				if (!beg[i] || !strcmp(beg[i], ";"))
					cont = 0;
				beg[i] = NULL;
				currcomm++;
				beg += i + 1;
				i = 0;
			}
			else
				i++;
		}
		// PARSING END
		// FORKING & PIPING & EXECUTION START
		if (strcmp(g_data.cmds[0].args[0], "cd"))
		{
			i = 0;
			while (i < g_data.cmdnum)
			{
				if (i != g_data.cmdnum - 1)
				{
					int	pipefd[2];
					if (pipe(pipefd))
						ft_exit("error: fatal\n", 0);
					g_data.cmds[i].out = pipefd[1];
					g_data.cmds[i + 1].in = pipefd[0];
				}
				g_data.family[i] = fork();
				if (g_data.family[i] == -1)
					ft_exit("error: fatal\n", 1);
				else if (g_data.family[i] == 0)
					ft_exec(g_data.cmds + i);
				waitpid(g_data.family[i], NULL, 0);
				if (g_data.cmds[i].in != 0)
					close(g_data.cmds[i].in);
				if (g_data.cmds[i].out != 1)
					close(g_data.cmds[i].out);
				i++;
			}
		}
		else
		{
			if (!g_data.cmds[0].args[1] || g_data.cmds[0].args[2])
				ft_puterr("error: cd: bad arguments\n");
			else
				if (chdir(g_data.cmds[0].args[1]) == -1)
				{
					ft_puterr("error: cd: cannot change directory to ");
					ft_puterr(g_data.cmds[0].args[1]);
					ft_puterr("\n");
				}
		}
		// FORKING & PIPING & EXECUTION END
		if (g_data.segments > 1)
		{
			i = 0;
			while (g_data.cmds[g_data.cmdnum - 1].args[i])
				i++;
			argv = &g_data.cmds[g_data.cmdnum - 1].args[i] + 1;
		}
		free(g_data.cmds);
		free(g_data.family);
		g_data.segments--;
	}
}
