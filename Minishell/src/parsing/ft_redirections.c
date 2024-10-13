/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_redirections.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mohamibr <mohamibr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 15:57:49 by mustafa-mac       #+#    #+#             */
/*   Updated: 2024/10/12 16:51:02 by mohamibr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/minishell.h"

int	search_for_redirection(t_token *token1)
{
	t_token	*token;

	token = token1;
	while (token)
	{
		if (token->token_type == REDIRECT_OUT || token->token_type == APPEND
			|| token->token_type == REDIRECT_IN)
		{
			return (1);
		}
		token = token->next;
	}
	return (0);
}

char	*get_filename(t_token *token)
{
	while (token)
	{
		if ((token->token_type == APPEND || token->token_type == REDIRECT_OUT
				|| token->token_type == REDIRECT_IN))
		{
			if (token->next == NULL)
				return (NULL);
			return (token->next->tokens);
		}
		token = token->next;
	}
	return (NULL);
}

void	remove_redirection_tokens(t_token **head)
{
	t_token	*current;
	t_token	*prev;
	t_token	*to_delete;
	t_token	*next_token;

	current = *head;
	prev = NULL;
	while (current)
	{
		if (current->token_type == REDIRECT_OUT || current->token_type == APPEND
			|| current->token_type == REDIRECT_IN)
		{
			to_delete = current;
			next_token = current->next;
			if (next_token == NULL)
				return ;
			if (prev)
				prev->next = next_token->next;
			else
				*head = next_token->next;
			free(to_delete->tokens);
			free(to_delete);
			if (next_token)
			{
				free(next_token->tokens);
				free(next_token);
			}
			current = (prev) ? prev->next : *head;
		}
		else
		{
			prev = current;
			current = current->next;
		}
	}
}

void	ft_trunck(t_token *token, t_env_cpy *env)
{
	int		fd;
	char	*file_name;

	file_name = get_filename(token);
	if (!file_name)
	{
		perror("filename");
		env->last_exit_status = 2;
		return ;
	}
	fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror("open");
		env->last_exit_status = 1;
		return ;
	}
	if (env->last_output_fd != -1)
		close(env->last_output_fd);
	env->last_output_fd = fd;
}

void	ft_append(t_token *token, t_env_cpy *env)
{
	int		fd;
	char	*file_name;

	file_name = get_filename(token);
	if (!file_name)
	{
		perror("filename");
		env->last_exit_status = 2;
		return ;
	}
	fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd == -1)
	{
		perror("open");
		env->last_exit_status = 1;
		return ;
	}
	if (env->last_output_fd != -1)
		close(env->last_output_fd);
	env->last_output_fd = fd;
}

void	ft_infile(t_token *token, t_env_cpy *env)
{
	int		fd;
	char	*file_name;

	file_name = get_filename(token);
	if (!file_name)
	{
		perror("filename");
		env->last_exit_status = 2;
		return ;
	}
	fd = open(file_name, O_RDONLY);
	if (fd == -1)
	{
		perror("open");
		env->last_exit_status = 1;
		return ;
	}
	if (env->last_input_fd != -1)
		close(env->last_input_fd);
	env->last_input_fd = fd;
}

// ft_redirections.c

void ft_redirection(t_token **token, t_env_cpy *env)
{
    t_token *head = *token;
    int stdout_backup;
    int stdin_backup;

    env->last_output_fd = -1;  // Ensure initialization
    env->last_input_fd = -1;   // Ensure initialization
    env->last_exit_status = 0; // Initialize the exit status

    stdout_backup = dup(STDOUT_FILENO);
    if (stdout_backup == -1)
    {
        perror("dup");
        env->last_exit_status = 1;
        return;
    }

    stdin_backup = dup(STDIN_FILENO);
    if (stdin_backup == -1)
    {
        perror("dup");
        close(stdout_backup); // Clean up before returning
        env->last_exit_status = 1;
        return;
    }

    while (head)
    {
        if (head->token_type == REDIRECT_OUT)
            ft_trunck(head, env);
        else if (head->token_type == APPEND)
            ft_append(head, env);
        else if (head->token_type == REDIRECT_IN)
            ft_infile(head, env);
        head = head->next;
    }

    remove_redirection_tokens(token);

    if (env->last_exit_status != 0)
    {
        dup2(stdout_backup, STDOUT_FILENO);
        close(stdout_backup);
        dup2(stdin_backup, STDIN_FILENO);
        close(stdin_backup);
        return;
    }

    if (env->last_input_fd != -1)
    {
        if (dup2(env->last_input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(stdout_backup);
            close(stdin_backup);
            env->last_exit_status = 1;
            return;
        }
        close(env->last_input_fd);
    }

    if (env->last_output_fd != -1)
    {
        if (dup2(env->last_output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(stdout_backup);
            close(stdin_backup);
            env->last_exit_status = 1;
            return;
        }
        close(env->last_output_fd);
    }

    if (env->last_exit_status == 0)
        ft_cmd(*token, env, 1); // Use the updated token list

    if (dup2(stdout_backup, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        env->last_exit_status = 1;
    }
    close(stdout_backup);

    if (dup2(stdin_backup, STDIN_FILENO) == -1)
    {
        perror("dup2");
        env->last_exit_status = 1;
    }
    close(stdin_backup);
}


int	check_token(t_token *head)
{
	t_token	*token;

	token = head;
	while (token)
	{
		if (token->token_type == PIPE && (token->next == NULL
				|| token->next->token_type == PIPE))
			return (1);
		else if (token && ((token->token_type == REDIRECT_IN)
				|| (token->token_type == REDIRECT_OUT)
				|| (token->token_type == APPEND)
				|| (token->token_type == HEREDOC)
				|| (token->token_type == PIPE)))
		{
			if (!token->next)
				return (1);
			else if (token->next && ((token->next->token_type == REDIRECT_IN)
					|| (token->next->token_type == REDIRECT_OUT)
					|| (token->next->token_type == APPEND)
					|| (token->next->token_type == HEREDOC)
					|| (token->next->token_type == PIPE)))
				return (1);
		}
		token = token->next;
	}
	return (0);
}

// tokenize_check.c

void check_redirections(t_token **token, t_env_cpy *env)
{
    if (*token && ((*token)->token_type == REDIRECT_IN
            || (*token)->token_type == REDIRECT_OUT
            || (*token)->token_type == APPEND
            || (*token)->token_type == HEREDOC))
    {
        if ((*token)->next && ((*token)->next->token_type == REDIRECT_IN
                || (*token)->next->token_type == REDIRECT_OUT
                || (*token)->next->token_type == APPEND
                || (*token)->next->token_type == HEREDOC))
        {
            perror((*token)->next->tokens);
            env->last_exit_status = 2;
            return;
        }
    }
    if (check_token(*token))
    {
        fprintf(stderr, "zsh: parse error near `\\n'\n");
        env->last_exit_status = 2;
        return;
    }
    ft_redirection(token, env);
}